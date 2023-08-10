#include <iostream>

#include "decoded_image.h"
#include "bvc.h"
#include "bvc_result.h"

#include "bvc_common/format.h"
#include "bvc_dec/picture.h"
#include "bvc_dec/nal.h"

BvcDecoder::BvcDecoder()
	: Params(nullptr), Decoder(nullptr)
{
}

BvcDecoder::~BvcDecoder()
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

DecodeResult* BvcDecoder::Init(DecoderConfig& InConfig)
{
	Config = InConfig;

	Decoder = new bvc_decoder();
	if (!Decoder)
	{
		std::cerr << "Error: Failed to allocate decoder" << std::endl;
		std::exit(-1);
	}

	return new BvcResult(Decoder->init());
}

DecodeResult* BvcDecoder::Decode(const uint8_t* InNalBytes, size_t InNalSize)
{
	bvc_dec_result Result;
	bvc_dec_nal	   Nal;
	Nal.size = InNalSize;
	Nal.bytes = new uint8_t[InNalSize];
	memcpy(Nal.bytes, InNalBytes, InNalSize);

	Result = Decoder->decode_nal(&Nal);
	if (Result != BVC_DEC_OK)
	{
		return new BvcResult(Result);
	}

	// Check if there is a decoded picture ready to be output.
	bvc_decoded_picture DecodedPicture;
	if (Decoder->get_picture(&DecodedPicture) == BVC_DEC_OK)
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

			if (DecodedPicture.info.format != bvc_chroma_format::BVC_CHROMA_FORMAT_UNDEFINED)
			{
				// clang-format off
				if       (DecodedPicture.info.format == bvc_chroma_format::BVC_CHROMA_FORMAT_MONOCHROME) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_MONOCHROME;
				} else if(DecodedPicture.info.format == bvc_chroma_format::BVC_CHROMA_FORMAT_420) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_420;
				} else if(DecodedPicture.info.format == bvc_chroma_format::BVC_CHROMA_FORMAT_422) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_422;
				} else if(DecodedPicture.info.format == bvc_chroma_format::BVC_CHROMA_FORMAT_444) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_444;
				}
				// clang-format on
			}

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new BvcResult(BVC_DEC_OK);
}
