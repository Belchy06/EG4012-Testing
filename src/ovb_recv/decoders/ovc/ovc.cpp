#include <iostream>

#include "ovb_recv/image/decoded_image.h"
#include "ovc.h"
#include "ovc_result.h"

#include "ovc_common/format.h"
#include "ovc_common/entropy/entropy.h"
#include "ovc_common/picture.h"
#include "ovc_common/nal.h"

#define LogOvcDecoder "LogOvcDecoder"

void OvcLogFunction(int level, const char* fmt, va_list args)
{
	std::vfprintf(stdout, fmt, args);
}

OvcDecoder::OvcDecoder()
	: Params(new ovc_dec_config()), Decoder(nullptr)
{
}

OvcDecoder::~OvcDecoder()
{
	if (Decoder)
	{
		Decoder = nullptr;
	}

	if (Params)
	{
		Params = nullptr;
	}
}

DecodeResult* OvcDecoder::Init(DecoderConfig& InConfig)
{
	Config = InConfig;

	Decoder = new ovc_decoder();
	if (!Decoder)
	{
		LOG(LogOvcDecoder, LOG_SEVERITY_ERROR, "Failed to allocate decoder");
		std::exit(-1);
	}

	Params->log_verbosity = static_cast<ovc_verbosity>(InConfig.LogLevel);
	Params->error_concealment = InConfig.OvcErrorConcealment;

	ovc_dec_result Result = Decoder->init(Params);

	Decoder->set_logging_callback(&::OvcLogFunction);

	return new OvcResult(Result);
}

DecodeResult* OvcDecoder::Decode(uint8_t* InNalBytes, size_t InNalSize)
{
	ovc_dec_result Result;
	ovc_nal		   Nal;
	Nal.size = InNalSize;
	Nal.bytes = new uint8_t[InNalSize];
	memcpy(Nal.bytes, InNalBytes, InNalSize);

	Result = Decoder->decode_nal(&Nal);
	if (Result != OVC_DEC_OK)
	{
		return new OvcResult(Result);
	}

	// Check if there is a decoded picture ready to be output.
	ovc_picture DecodedPicture;
	if (Decoder->get_picture(&DecodedPicture) == OVC_DEC_OK)
	{
		if (OnDecodedImageCallback != nullptr)
		{
			DecodedImage Image;

			if (DecodedPicture.format != ovc_chroma_format::OVC_CHROMA_FORMAT_UNDEFINED)
			{
				// clang-format off
				if       (DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_MONOCHROME) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_400;
				} else if(DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_420) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_420;
				} else if(DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_422) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_422;
				} else if(DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_444) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_444;
				}
				// clang-format on
			}

			std::vector<uint8_t> ImageBytes;
			//                                      Y
			// clang-format off
			size_t FrameSize = (DecodedPicture.planes[0].width * DecodedPicture.planes[0].height)  // Y
                             + (DecodedPicture.planes[1].width * DecodedPicture.planes[1].height)  // U
                             + (DecodedPicture.planes[2].width * DecodedPicture.planes[2].height); // V
			// clang-format on
			ImageBytes.reserve(FrameSize);

			for (size_t c = 0; c < (size_t)(Image.Config.Format == CHROMA_FORMAT_400 ? 1 : 3); c++)
			{
				ovc_plane			 Plane = DecodedPicture.planes[c];
				std::vector<uint8_t> PlaneVec;
				PlaneVec.reserve(Plane.width * Plane.height);
				for (size_t y = 0; y < Plane.height; y++)
				{
					for (size_t x = 0; x < Plane.width; x++)
					{
						PlaneVec.push_back(Plane.data[x + y * Plane.width]);
					}
				}
				ImageBytes.insert(ImageBytes.end(), PlaneVec.begin(), PlaneVec.end());
			}

			Image.Bytes = ImageBytes;
			Image.Size = ImageBytes.size();
			Image.Config.BitDepth = DecodedPicture.planes[0].bit_depth;
			Image.Config.Width = DecodedPicture.planes[0].width;
			Image.Config.Height = DecodedPicture.planes[0].height;

			if (static_cast<int>(DecodedPicture.framerate) == DecodedPicture.framerate)
			{
				Image.Config.FramerateNum = DecodedPicture.framerate;
				Image.Config.FramerateDenom = 1;
			}
			else
			{
				Image.Config.FramerateNum = static_cast<int>(DecodedPicture.framerate * 1000);
				Image.Config.FramerateDenom = 1000;
			}

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new OvcResult(OVC_DEC_OK);
}

#undef LogOvcDecoder