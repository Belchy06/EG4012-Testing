#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "ovb_common/common.h"
#include "avc_encoder.h"
#include "avc_result.h"
#include "wels/codec_app_def.h"

#define LogAvcEncoder "LogAvcEncoder"

AvcEncoder::AvcEncoder()
	: Params(new SEncParamExt()), Encoder(nullptr)
{
	memset(Params, 0, sizeof(SEncParamExt));
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

	Encoder->GetDefaultParams(Params);

	Params->iUsageType = CAMERA_VIDEO_REAL_TIME;
	Params->fMaxFrameRate = InConfig.Framerate;
	Params->iPicWidth = InConfig.Width;
	Params->iPicHeight = InConfig.Height;
	Params->uiIntraPeriod = InConfig.AvcIntraPeriod;
	Params->iNumRefFrame = InConfig.AvcNumRefFrame;

	Params->sSpatialLayers[0].iVideoWidth = InConfig.Width;
	Params->sSpatialLayers[0].iVideoHeight = InConfig.Height;
	Params->sSpatialLayers[0].fFrameRate = InConfig.Framerate;

	if (InConfig.AvcQP != -1)
	{
		Params->iMinQp = InConfig.AvcQP;
		Params->iMaxQp = InConfig.AvcQP;
		Params->iRCMode = RC_QUALITY_MODE;
		Params->iTargetBitrate = 10000000;
	}

	if (InConfig.AvcBitrate != -1)
	{
		Params->iTargetBitrate = InConfig.AvcBitrate;
		Params->sSpatialLayers[0].iSpatialBitrate = InConfig.AvcBitrate;
		Params->iRCMode = RC_BITRATE_MODE;
	}

	Result = Encoder->InitializeExt(Params);
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
		int Height = c == 0 ? Config.Height : ScaleY(Config.Height, Config.Format);

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
	if (Result != cmResultSuccess)
	{
		return new AvcResult(Result);
	}

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