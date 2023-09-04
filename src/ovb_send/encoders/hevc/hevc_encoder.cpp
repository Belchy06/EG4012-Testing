#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "ovb_common/common.h"
#include "hevc_encoder.h"
#include "hevc_result.h"
#include "libde265/image.h"

#define LogHevcEncoder "LogHevcEncoder"

HevcEncoder::HevcEncoder()
	: Encoder(nullptr)
{
}

HevcEncoder::~HevcEncoder()
{
	if (Encoder)
	{
		en265_free_encoder(Encoder);
	}
}

EncodeResult* HevcEncoder::Init(EncoderConfig& InConfig)
{
	Config = InConfig;

	Encoder = en265_new_encoder();
	if (Encoder == nullptr)
	{
		LOG(LogHevcEncoder, LOG_SEVERITY_ERROR, "Failed to allocate encoder");
		std::exit(-1);
	}

	en265_show_parameters(Encoder);

	de265_error Result = en265_set_parameter_choice(Encoder, "sop-structure", Config.HevcSopStructure.c_str());
	if (Result != DE265_OK)
	{
		LOG(LogHevcEncoder, LOG_SEVERITY_ERROR, "Failed to set encoder sop-structure");
		std::exit(-1);
	}

	Result = en265_set_parameter_int(Encoder, "CTB-QScale-Constant", Config.HevcQP);
	if (Result != DE265_OK)
	{
		LOG(LogHevcEncoder, LOG_SEVERITY_ERROR, "Failed to set encoder qp");
		std::exit(-1);
	}

	// TODO (belchy06):
	// de265_set_verbosity();
	Result = en265_start_encoder(Encoder, 0);
	if (Result != DE265_OK)
	{
		LOG(LogHevcEncoder, LOG_SEVERITY_ERROR, "Failed to start encoder");
		std::exit(-1);
	}

	return new HevcResult(DE265_OK);
}

EncodeResult* HevcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	de265_chroma Format = de265_chroma_mono;
	switch (Config.Format)
	{
		case CHROMA_FORMAT_400:
			Format = de265_chroma_mono;
			break;
		case CHROMA_FORMAT_420:
			Format = de265_chroma_420;
			break;
		case CHROMA_FORMAT_422:
			Format = de265_chroma_422;
			break;
		case CHROMA_FORMAT_444:
			Format = de265_chroma_444;
			break;
	}

	de265_image* InputImage = en265_allocate_image(Encoder, Config.Width, Config.Height, Format, /*NULL,*/ 0, NULL);

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

		de265_alloc_image_plane(InputImage, c, PlanePtr - Width * Height, Width, NULL);
	}

	de265_error Result;
	if (bInLastPicture)
	{
		Result = en265_push_eof(Encoder);
	}
	else
	{
		Result = en265_push_image(Encoder, InputImage);
	}

	if (Result != DE265_OK)
	{
		return new HevcResult(Result);
	}

	Result = en265_encode(Encoder);

	if (OnEncodedImageCallback != nullptr)
	{
		std::vector<NALU> NALUs;
		for (;;)
		{
			en265_packet* Packet = en265_get_packet(Encoder, 0);
			if (Packet == NULL)
			{
				break;
			}

			NALUs.push_back(NALU(Packet->data, Packet->length));
		}

		OnEncodedImageCallback->OnEncodeComplete(NALUs);
	}

	return new HevcResult(DE265_OK);
}

#undef LogHevcEncoder