#include <iostream>

#include "ovb_common/image/decoded_image.h"
#include "xvc_decoder.h"
#include "xvc_result.h"

#define LogXvcDecoder "LogXvcDecoder"

XvcDecoder::XvcDecoder()
	: Api(xvc_decoder_api_get())
	, Params(Api->parameters_create())
	, Decoder(nullptr)
{
}

XvcDecoder::~XvcDecoder()
{
	if (Decoder)
	{
		Api->decoder_destroy(Decoder);
		Decoder = nullptr;
	}

	if (Params)
	{
		Api->parameters_destroy(Params);
		Params = nullptr;
	}
}

DecodeResult* XvcDecoder::Init(DecoderConfig& InConfig)
{
	Config = InConfig;
	Api->parameters_set_default(Params);

	xvc_dec_return_code Result = Api->parameters_check(Params);
	if (Result != XVC_DEC_OK)
	{
		return new XvcResult(Result);
	}

	Decoder = Api->decoder_create(Params);
	if (!Decoder)
	{
		LOG(LogXvcDecoder, LOG_SEVERITY_ERROR, "Failed to allocate decoder");
		std::exit(-1);
	}

	return new XvcResult(Result);
}

DecodeResult* XvcDecoder::Decode(uint8_t* InNalBytes, size_t InNalSize)
{
	xvc_dec_return_code Result;
	Result = Api->decoder_decode_nal(Decoder, InNalBytes, InNalSize, 0);
	if (Result != XVC_DEC_OK)
	{
		return new XvcResult(Result);
	}

	// Check if there is a decoded picture ready to be output.
	xvc_decoded_picture DecodedPicture;
	if (Api->decoder_get_picture(Decoder, &DecodedPicture) == XVC_DEC_OK)
	{
		if (OnDecodedImageCallback != nullptr)
		{
			DecodedImage Image;
			if (DecodedPicture.stats.chroma_format != xvc_dec_chroma_format::XVC_DEC_CHROMA_FORMAT_UNDEFINED)
			{
				// clang-format off
				if       (DecodedPicture.stats.chroma_format == xvc_dec_chroma_format::XVC_DEC_CHROMA_FORMAT_MONOCHROME) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_400;
				} else if(DecodedPicture.stats.chroma_format == xvc_dec_chroma_format::XVC_DEC_CHROMA_FORMAT_420) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_420;
				} else if(DecodedPicture.stats.chroma_format == xvc_dec_chroma_format::XVC_DEC_CHROMA_FORMAT_422) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_422;
				} else if(DecodedPicture.stats.chroma_format == xvc_dec_chroma_format::XVC_DEC_CHROMA_FORMAT_444) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_444;
				}
				// clang-format on
			}

			Image.Bytes.reserve(DecodedPicture.size);
			for (size_t c = 0; c < (size_t)(DecodedPicture.stats.chroma_format == XVC_DEC_CHROMA_FORMAT_MONOCHROME ? 1 : 3); c++)
			{
				const char* Plane = DecodedPicture.planes[c];

				int Width = c == 0 ? DecodedPicture.stats.width : ScaleX(DecodedPicture.stats.width, Image.Config.Format);
				int Height = c == 0 ? DecodedPicture.stats.height : ScaleY(DecodedPicture.stats.height, Image.Config.Format);

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

			Image.Size = DecodedPicture.size;
			Image.Config.BitDepth = DecodedPicture.stats.bitdepth;
			Image.Config.Width = DecodedPicture.stats.width;
			Image.Config.Height = DecodedPicture.stats.height;

			if (static_cast<int>(DecodedPicture.stats.framerate) == DecodedPicture.stats.framerate)
			{
				Image.Config.FramerateNum = DecodedPicture.stats.framerate;
				Image.Config.FramerateDenom = 1;
			}
			else
			{
				Image.Config.FramerateNum = static_cast<int>(DecodedPicture.stats.framerate * 1000);
				Image.Config.FramerateDenom = 1000;
			}

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new XvcResult(XVC_DEC_OK);
}

int XvcDecoder::ScaleX(int InX, EChromaFormat InFormat)
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

int XvcDecoder::ScaleY(int InY, EChromaFormat InFormat)
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

#undef LogXvcDecoder