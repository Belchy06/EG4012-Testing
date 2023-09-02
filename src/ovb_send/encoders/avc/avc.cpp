#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "ovb_common/common.h"
#include "avc.h"
#include "avc_result.h"
#include "wels/codec_app_def.h"

#define LogAvcEncoder "LogAvcEncoder"

AvcEncoder::AvcEncoder()
	: Params(new SEncParamBase()), Encoder(nullptr)
{
	memset(Params, 0, sizeof(SEncParamBase));
}

AvcEncoder::~AvcEncoder()
{
	if (Encoder)
	{
		Encoder->Uninitialize();
		WelsDestroySVCEncoder(Encoder);
		Encoder = nullptr;
	}

	if (Params)
	{
		Params = nullptr;
	}
}

EncodeResult* AvcEncoder::Init(EncoderConfig& InConfig)
{
	Config = InConfig;

	int32_t Result = WelsCreateSVCEncoder(&Encoder);
	if (Result != 0)
	{
		LOG(LogAvcEncoder, LOG_SEVERITY_ERROR, "Failed to allocate encoder");
		std::exit(-1);
	}

	Params->iUsageType = CAMERA_VIDEO_REAL_TIME;
	Params->fMaxFrameRate = InConfig.Framerate;
	Params->iPicWidth = InConfig.Width;
	Params->iPicHeight = InConfig.Height;
	Params->iTargetBitrate = UNSPECIFIED_BIT_RATE;

	Result = Encoder->Initialize(Params);
	if (Result != 0)
	{
		LOG(LogAvcEncoder, LOG_SEVERITY_ERROR, "Failed to allocate encoder");
		std::exit(-1);
	}

	int videoFormat = videoFormatI420;
	Result = Encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
	if (Result != 0)
	{
		LOG(LogAvcEncoder, LOG_SEVERITY_ERROR, "Failed to allocate encoder");
		std::exit(-1);
	}

	return new AvcResult(cmResultSuccess);
}

EncodeResult* AvcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	SFrameBSInfo Info;
	memset(&Info, 0, sizeof(SFrameBSInfo));
	SSourcePicture Pic;
	memset(&Pic, 0, sizeof(SSourcePicture));

	Pic.iPicWidth = Config.Width;
	Pic.iPicHeight = Config.Height;
	Pic.iColorFormat = videoFormatI420;

	uint8_t* SrcBytes = InPictureBytes.data();
	int		 NumComponents = Config.Format == EChromaFormat::CHROMA_FORMAT_400 ? 1 : 3;
	for (int c = 0; c < NumComponents; c++)
	{
		int Width = c == 0 ? Config.Width : ScaleX(Config.Width, Config.Format);
		int Height = c == 0 ? Config.Height : ScaleX(Config.Height, Config.Format);

		uint8_t* PlanePtr = new uint8_t[Width * Height]{ 0 };

		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				PlanePtr[x] = SrcBytes[x];
			}
			SrcBytes += Width;
			PlanePtr += Width;
		}

		Pic.iStride[c] = Width;
		Pic.pData[c] = PlanePtr - Width * Height;
	}

	CM_RETURN Result = cmResultSuccess;
	Result = (CM_RETURN)Encoder->ForceIntraFrame(true);
	Result = (CM_RETURN)Encoder->EncodeFrame(&Pic, &Info);

	if (Result != cmResultSuccess)
	{
		return new AvcResult(Result);
	}

	if (OnEncodedImageCallback != nullptr)
	{
		std::vector<NALU> NALUs;
		// There should only ever be 1 layer as we aren't doing SVC
		for (int i = 0; i < Info.iLayerNum; i++)
		{
			SLayerBSInfo BSInfo = Info.sLayerInfo[i];

			size_t Len = 0;
			for (int j = 0; j < BSInfo.iNalCount; j++)
			{
				int NALSize = BSInfo.pNalLengthInByte[j];
				NALUs.push_back(NALU(BSInfo.pBsBuf + Len, NALSize));

				Len += NALSize;
			}
		}

		OnEncodedImageCallback->OnEncodeComplete(NALUs);
	}

	return new AvcResult(cmResultSuccess);
}

#undef LogAvcEncoder