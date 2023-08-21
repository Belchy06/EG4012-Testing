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

	Params->entropy_coder = ovc_entropy_coder::OVC_ENTROPY_CODER_ARITHMETIC;

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
	ovc_nal* NalUnits;
	size_t	 NumNalUnits;

	ovc_picture* Input = new ovc_picture();
	Input->info.width = Config.Width;
	Input->info.height = Config.Height;

	int		 NumComponents = Config.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME ? 1 : 3;
	uint8_t* SrcBytes = InPictureBytes.data();
	int		 FrameSize = Config.Width * Config.Height;
	for (int c = 0; c < NumComponents; c++)
	{
		const EYuvComponent Component = static_cast<EYuvComponent>(c);
		if (Component == EYuvComponent::Y)
		{
			Input->Y = new uint8_t[FrameSize]{ 0 };
			memcpy(Input->Y, SrcBytes, FrameSize);
		}
		else if (Component == EYuvComponent::U)
		{
			switch (Config.Format)
			{
				case EChromaFormat::CHROMA_FORMAT_420:
					Input->U = new uint8_t[FrameSize / 4]{ 0 };
					memcpy(Input->U, SrcBytes + FrameSize, (FrameSize) / 4);
					break;
				case EChromaFormat::CHROMA_FORMAT_422:
					Input->U = new uint8_t[(Config.Width / 2) * Config.Height]{ 0 };
					memcpy(Input->U, SrcBytes + FrameSize, (Config.Width / 2) * Config.Height);
					break;
				case EChromaFormat::CHROMA_FORMAT_444:
					Input->U = new uint8_t[FrameSize]{ 0 };
					memcpy(Input->U, SrcBytes + FrameSize, FrameSize);
					break;
				case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
				case EChromaFormat::CHROMA_FORMAT_UNDEFINED:
				default:
					break;
			}
		}
		else if (Component == EYuvComponent::V)
		{
			switch (Config.Format)
			{
				case EChromaFormat::CHROMA_FORMAT_420:
					Input->V = new uint8_t[FrameSize / 4]{ 0 };
					memcpy(Input->V, SrcBytes + (5 / 4) * FrameSize, (FrameSize) / 4);
					break;
				case EChromaFormat::CHROMA_FORMAT_422:
					Input->V = new uint8_t[(Config.Width / 2) * Config.Height]{ 0 };
					memcpy(Input->V, SrcBytes + (Config.Width * Config.Width) + (3 * Config.Width * Config.Height) + (2 * Config.Height * Config.Height), (Config.Width / 2) * Config.Height);
					break;
				case EChromaFormat::CHROMA_FORMAT_444:
					Input->V = new uint8_t[FrameSize]{ 0 };
					memcpy(Input->V, SrcBytes + 2 * FrameSize, FrameSize);
					break;
				case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
				case EChromaFormat::CHROMA_FORMAT_UNDEFINED:
				default:
					break;
			}
		}
	}

	ovc_enc_result Result = Encoder->encode(Input, &NalUnits, &NumNalUnits);

	// Loop through all Nal Units that were received and write to file
	// the Nal Unit length followed by the actual Nal Unit.
	for (size_t i = 0; i < NumNalUnits; i++)
	{
		if (OnEncodedImageCallback != nullptr)
		{
			OnEncodedImageCallback->OnEncodeComplete(NalUnits[i].bytes, NalUnits[i].size);
		}
	}

	return new OvcResult(Result);
}