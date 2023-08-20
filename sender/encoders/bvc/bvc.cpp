#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "common.h"
#include "ovc.h"
#include "ovc_common/format.h"
#include "ovc_common/entropy.h"
#include "ovc_result.h"

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

	if (Config.Format != EChromaFormat::CHROMA_FORMAT_UNDEFINED)
	{
		// clang-format off
        if(Config.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME) {
            Params->format = ovc_chroma_format::OVC_CHROMA_FORMAT_MONOCHROME;
        } else if(Config.Format == EChromaFormat::CHROMA_FORMAT_420) {
            Params->format = ovc_chroma_format::OVC_CHROMA_FORMAT_420;
        } else if(Config.Format == EChromaFormat::CHROMA_FORMAT_422) {
            Params->format = ovc_chroma_format::OVC_CHROMA_FORMAT_422;
        } else if(Config.Format == EChromaFormat::CHROMA_FORMAT_444) {
            Params->format = ovc_chroma_format::OVC_CHROMA_FORMAT_444;
        }
		// clang-format on
	}

	Params->entropy_coder = ovc_entropy::OVC_ENTROPY_CODER_CABAC;

	Encoder = new ovc_encoder();
	if (!Encoder)
	{
		std::cerr << "Error: Failed to allocate encoder" << std::endl;
		std::exit(-1);
	}

	return new OvcResult(Encoder->init(Params));
}

EncodeResult* OvcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	ovc_enc_nal* NalUnits;
	int			 NumNalUnits;

	ovc_enc_result Result = Encoder->encode(&InPictureBytes[0], &NalUnits, &NumNalUnits);

	// Loop through all Nal Units that were received and write to file
	// the Nal Unit length followed by the actual Nal Unit.
	for (int i = 0; i < NumNalUnits; i++)
	{
		if (OnEncodedImageCallback != nullptr)
		{
			OnEncodedImageCallback->OnEncodeComplete(NalUnits[i].bytes, NalUnits[i].size);
		}
	}

	return new OvcResult(Result);
}