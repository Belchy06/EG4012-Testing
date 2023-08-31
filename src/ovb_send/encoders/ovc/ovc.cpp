#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "ovb_common/common.h"
#include "ovc.h"
#include "ovc_common/format.h"
#include "ovc_common/entropy/entropy.h"
#include "ovc_common/picture.h"
#include "ovc_result.h"

#define LogOvcEncoder "LogOvcEncoder"

void OvcLogFunction(int level, const char* fmt, va_list args)
{
	std::vfprintf(stdout, fmt, args);
}

OvcEncoder::OvcEncoder()
	: Params(new ovc_enc_config()), Encoder(nullptr)
{
}

OvcEncoder::~OvcEncoder()
{
	if (Encoder)
	{
		Encoder = nullptr;
	}

	if (Params)
	{
		Params = nullptr;
	}
}

EncodeResult* OvcEncoder::Init(EncoderConfig& InConfig)
{
	Config = InConfig;

	if (Config.Width)
	{
		Params->width = Config.Width;
	}

	if (Config.Height)
	{
		Params->height = Config.Height;
	}

	if (Config.Format != CHROMA_FORMAT_UNDEFINED)
	{
		// clang-format off
        if(Config.Format == CHROMA_FORMAT_400) {
            Params->format = OVC_CHROMA_FORMAT_MONOCHROME;
        } else if(Config.Format == CHROMA_FORMAT_420) {
            Params->format = OVC_CHROMA_FORMAT_420;
        } else if(Config.Format == CHROMA_FORMAT_422) {
            Params->format = OVC_CHROMA_FORMAT_422;
        } else if(Config.Format == CHROMA_FORMAT_444) {
            Params->format = OVC_CHROMA_FORMAT_444;
        }
		// clang-format on
	}

	Params->repeat_vps = InConfig.OvcRepeatVPS;

	Params->wavelet_family = InConfig.OvcWaveletFamily;
	Params->wavelet_config = InConfig.OvcWaveletConfig;

	Params->partition_type = InConfig.OvcPartitionType;
	Params->num_levels = InConfig.OvcNumLevels;
	Params->num_parts_exp = InConfig.OvcNumPartsExp;

	Params->spiht = InConfig.OvcSPIHT;
	Params->bits_per_pixel = InConfig.OvcBitsPerPixel;

	Params->entropy_coder = InConfig.OvcEntropyCoder;

	Encoder = new ovc_encoder();
	if (!Encoder)
	{
		LOG(LogOvcEncoder, LOG_SEVERITY_ERROR, "Failed to allocate encoder");
		std::exit(-1);
	}

	ovc_enc_result Result = Encoder->init(Params);

	Encoder->set_logging_callback(&::OvcLogFunction);

	return new OvcResult(Result);
}

EncodeResult* OvcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	ovc_nal* NalUnits;
	size_t	 NumNalUnits;

	ovc_picture* Input = new ovc_picture();

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

		ovc_plane Plane = ovc_plane();
		Plane.data = PlanePtr - Width * Height;
		Plane.width = Width;
		Plane.height = Height;
		Input->planes[c] = std::move(Plane);
	}

	ovc_enc_result Result = Encoder->encode(Input, &NalUnits, &NumNalUnits);

	size_t TotalSize = 0;
	for (size_t i = 0; i < NumNalUnits; i++)
	{
		TotalSize += NalUnits[i].size;
	}

	LOG(LogOvcEncoder, LOG_SEVERITY_DETAILS, "Encoded size {}", TotalSize);

	// Loop through all Nal Units that were received and write to file
	// the Nal Unit length followed by the actual Nal Unit.

	std::vector<NALU> NALUs;
	for (size_t i = 0; i < NumNalUnits; i++)
	{
		NALUs.push_back(NALU(NalUnits[i].bytes, NalUnits[i].size));
	}

	if (OnEncodedImageCallback != nullptr)
	{
		OnEncodedImageCallback->OnEncodeComplete(NALUs);
	}

	return new OvcResult(Result);
}

#undef LogOvcEncoder