#include <iostream>

#include "ovb_recv/image/decoded_image.h"
#include "ovc.h"
#include "ovc_result.h"

#include "ovc_common/format.h"
#include "ovc_common/entropy/entropy.h"
#include "ovc_common/picture.h"
#include "ovc_common/nal.h"

OvcDecoder::OvcDecoder()
	: Params(nullptr), Decoder(nullptr)
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
		std::cerr << "Error: Failed to allocate decoder" << std::endl;
		std::exit(-1);
	}

	return new OvcResult(Decoder->init());
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
			// TODO (belchy06): This will need to change if there's more than 1 plane
			Image.Bytes.resize(DecodedPicture.planes[0].width * DecodedPicture.planes[0].height);
			memcpy(Image.Bytes.data(), DecodedPicture.planes[0].data, DecodedPicture.planes[0].width * DecodedPicture.planes[0].height);
			Image.Size = DecodedPicture.planes[0].width * DecodedPicture.planes[0].height;
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

			if (DecodedPicture.format != ovc_chroma_format::OVC_CHROMA_FORMAT_UNDEFINED)
			{
				// clang-format off
				if       (DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_MONOCHROME) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_MONOCHROME;
				} else if(DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_420) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_420;
				} else if(DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_422) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_422;
				} else if(DecodedPicture.format == ovc_chroma_format::OVC_CHROMA_FORMAT_444) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_444;
				}
				// clang-format on
			}

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new OvcResult(OVC_DEC_OK);
}
