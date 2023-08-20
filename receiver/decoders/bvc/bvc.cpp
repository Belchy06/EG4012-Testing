#include <iostream>

#include "decoded_image.h"
#include "ovc.h"
#include "ovc_result.h"

#include "ovc_common/format.h"
#include "ovc_common/entropy.h"
#include "ovc_dec/picture.h"
#include "ovc_dec/nal.h"

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

DecodeResult* OvcDecoder::Decode(const uint8_t* InNalBytes, size_t InNalSize)
{
	ovc_dec_result Result;
	ovc_dec_nal	   Nal;
	Nal.size = InNalSize;
	Nal.bytes = new uint8_t[InNalSize];
	memcpy(Nal.bytes, InNalBytes, InNalSize);

	Result = Decoder->decode_nal(&Nal);
	if (Result != OVC_DEC_OK)
	{
		return new OvcResult(Result);
	}

	// Check if there is a decoded picture ready to be output.
	ovc_decoded_picture DecodedPicture;
	if (Decoder->get_picture(&DecodedPicture) == OVC_DEC_OK)
	{
		if (OnDecodedImageCallback != nullptr)
		{
			DecodedImage Image;
			Image.Bytes.resize(DecodedPicture.size);
			memcpy(Image.Bytes.data(), DecodedPicture.bytes, DecodedPicture.size);
			Image.Size = DecodedPicture.size;
			Image.Config.BitDepth = DecodedPicture.info.bit_depth;
			Image.Config.Width = DecodedPicture.info.width;
			Image.Config.Height = DecodedPicture.info.height;

			if (static_cast<int>(DecodedPicture.info.framerate) == DecodedPicture.info.framerate)
			{
				Image.Config.FramerateNum = DecodedPicture.info.framerate;
				Image.Config.FramerateDenom = 1;
			}
			else
			{
				Image.Config.FramerateNum = static_cast<int>(DecodedPicture.info.framerate * 1000);
				Image.Config.FramerateDenom = 1000;
			}

			if (DecodedPicture.info.format != ovc_chroma_format::OVC_CHROMA_FORMAT_UNDEFINED)
			{
				// clang-format off
				if       (DecodedPicture.info.format == ovc_chroma_format::OVC_CHROMA_FORMAT_MONOCHROME) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_MONOCHROME;
				} else if(DecodedPicture.info.format == ovc_chroma_format::OVC_CHROMA_FORMAT_420) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_420;
				} else if(DecodedPicture.info.format == ovc_chroma_format::OVC_CHROMA_FORMAT_422) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_422;
				} else if(DecodedPicture.info.format == ovc_chroma_format::OVC_CHROMA_FORMAT_444) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_444;
				}
				// clang-format on
			}

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new OvcResult(OVC_DEC_OK);
}
