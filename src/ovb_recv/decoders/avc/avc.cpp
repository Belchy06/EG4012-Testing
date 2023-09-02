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

#define LogAvcDecoder "LogAvcDecoder"

AvcDecoder::AvcDecoder()
	: Params(new SDecodingParam()), Decoder(nullptr)
{
	memset(Params, 0, sizeof(SDecodingParam));
}

AvcDecoder::~AvcDecoder()
{
	if (Decoder)
	{
		Decoder->Uninitialize();
		WelsDestroyDecoder(Decoder);
		Decoder = nullptr;
	}

	if (Params)
	{
		Params = nullptr;
	}
}

DecodeResult* AvcDecoder::Init(DecoderConfig& InConfig)
{
	Config = InConfig;

	int32_t Result = WelsCreateDecoder(&Decoder);
	if (Result != 0)
	{
		LOG(LogAvcDecoder, LOG_SEVERITY_ERROR, "Failed to allocate decoder");
		std::exit(-1);
	}

	Params->sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;

	Decoder->Initialize(Params);

	return new AvcResult(cmResultSuccess);
}

DecodeResult* AvcDecoder::Decode(uint8_t* InNalBytes, size_t InNalSize)
{
	uint8_t* Picture = new uint8_t[3];

	SBufferInfo DstBufInfo;
	memset(&DstBufInfo, 0, sizeof(SBufferInfo));

	CM_RETURN Result = (CM_RETURN)Decoder->DecodeFrameNoDelay(InNalBytes, (int)InNalSize, &Picture, &DstBufInfo);
	if (Result != cmResultSuccess)
	{
		return new AvcResult(Result);
	}

	if (DstBufInfo.iBufferStatus == 1)
	{
		if (OnDecodedImageCallback != nullptr)
		{
			DecodedImage Image;

			if (DstBufInfo.UsrData.sSystemBuffer.iFormat != 0)
			{
				// clang-format off
				if(DstBufInfo.UsrData.sSystemBuffer.iFormat == (int)videoFormatI420) {
					Image.Config.Format = CHROMA_FORMAT_420;
				}
				// clang-format on
			}

			std::vector<uint8_t> ImageBytes;
			//                                      Y
			// clang-format off
			size_t FrameSize = (DstBufInfo.UsrData.sSystemBuffer.iWidth * DstBufInfo.UsrData.sSystemBuffer.iHeight)  // Y
                             + (ScaleX(DstBufInfo.UsrData.sSystemBuffer.iWidth, Image.Config.Format) * ScaleY(DstBufInfo.UsrData.sSystemBuffer.iHeight, Image.Config.Format))  // U
                             + (ScaleX(DstBufInfo.UsrData.sSystemBuffer.iWidth, Image.Config.Format) * ScaleY(DstBufInfo.UsrData.sSystemBuffer.iHeight, Image.Config.Format)); // V
			// clang-format on
			ImageBytes.reserve(FrameSize);

			for (size_t c = 0; c < (size_t)(Image.Config.Format == CHROMA_FORMAT_400 ? 1 : 3); c++)
			{
				unsigned char* Plane = DstBufInfo.pDst[c];

				int Width = c == 0 ? DstBufInfo.UsrData.sSystemBuffer.iWidth : ScaleX(DstBufInfo.UsrData.sSystemBuffer.iWidth, Image.Config.Format);
				int Height = c == 0 ? DstBufInfo.UsrData.sSystemBuffer.iHeight : ScaleY(DstBufInfo.UsrData.sSystemBuffer.iHeight, Image.Config.Format);

				std::vector<uint8_t> PlaneVec;
				PlaneVec.reserve(Width * Height);
				for (int y = 0; y < Height; y++)
				{
					for (int x = 0; x < Width; x++)
					{
						PlaneVec.push_back(Plane[x + y * Width]);
					}
				}
				Image.Bytes.insert(Image.Bytes.end(), PlaneVec.begin(), PlaneVec.end());
			}

			Image.Bytes = ImageBytes;
			Image.Size = ImageBytes.size();
			Image.Config.BitDepth = 8;
			Image.Config.Width = DstBufInfo.UsrData.sSystemBuffer.iWidth;
			Image.Config.Height = DstBufInfo.UsrData.sSystemBuffer.iHeight;

			Image.Config.FramerateNum = 29997;
			Image.Config.FramerateDenom = 1000;

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new AvcResult(cmResultSuccess);
}

int AvcDecoder::ScaleX(int InX, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case CHROMA_FORMAT_400:
			return 0;
		case CHROMA_FORMAT_444:
			return InX;
		case CHROMA_FORMAT_420:
		case CHROMA_FORMAT_422:
			return InX >> 1;
		default:
			return 0;
	}
}

int AvcDecoder::ScaleY(int InY, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case CHROMA_FORMAT_400:
			return 0;
		case CHROMA_FORMAT_444:
		case CHROMA_FORMAT_422:
			return InY;
		case CHROMA_FORMAT_420:
			return InY >> 1;
		default:
			return 0;
	}
}