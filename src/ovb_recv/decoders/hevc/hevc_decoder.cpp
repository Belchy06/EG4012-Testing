#include <iterator>
#include <iostream>

#include "ovb_recv/decoders/hevc/hevc_decoder.h"
#include "ovb_recv/decoders/hevc/hevc_result.h"

#define LogHevcDecoder "LogHevcDecoder"

HevcDecoder::HevcDecoder()
	: Decoder(nullptr)
{
}

HevcDecoder::~HevcDecoder()
{
	if (Decoder)
	{
		Decoder = nullptr;
	}
}

DecodeResult* HevcDecoder::Init(DecoderConfig& InConfig)
{
	Config = InConfig;

	Decoder = de265_new_decoder();
	if (Decoder == nullptr)
	{
		LOG(LogHevcDecoder, LOG_SEVERITY_ERROR, "Failed to allocate decoder");
		std::exit(-1);
	}

	return new HevcResult(DE265_OK);
}

DecodeResult* HevcDecoder::Decode(uint8_t* InNalBytes, size_t InNalSize)
{
	de265_error Result = de265_decode_data(Decoder, InNalBytes, InNalSize);
	if (Result != DE265_OK)
	{
		return new HevcResult(Result);
	}

	const de265_image* OutputImage = de265_get_next_picture(Decoder);
	if (OutputImage != nullptr)
	{
		if (OnDecodedImageCallback != nullptr)
		{
			DecodedImage Image;

			// clang-format off
			if       (de265_get_chroma_format(OutputImage) == de265_chroma_mono) {
				Image.Config.Format = CHROMA_FORMAT_400;
			} else if(de265_get_chroma_format(OutputImage) == de265_chroma_420) {
				Image.Config.Format = CHROMA_FORMAT_420;
			} else if(de265_get_chroma_format(OutputImage) == de265_chroma_422) {
				Image.Config.Format = CHROMA_FORMAT_422;
			} else if(de265_get_chroma_format(OutputImage) == de265_chroma_444) {
				Image.Config.Format = CHROMA_FORMAT_444;
			}

			// clang-format on
			int FrameWidth = de265_get_image_width(OutputImage, 0);
			int FrameHeight = de265_get_image_height(OutputImage, 0);
			//                                      Y
			// clang-format off
			size_t FrameSize = (FrameWidth * FrameHeight)                                                            // Y
                             + (ScaleX(FrameWidth, Image.Config.Format) * ScaleY(FrameHeight, Image.Config.Format))  // U
                             + (ScaleX(FrameWidth, Image.Config.Format) * ScaleY(FrameHeight, Image.Config.Format)); // V
			// clang-format on
			Image.Bytes.reserve(FrameSize);

			uint8_t* ImageBytes = new uint8_t[FrameSize]{ 0 };

			for (size_t c = 0; c < (size_t)(Image.Config.Format == CHROMA_FORMAT_400 ? 1 : 3); c++)
			{
				int			   Stride = 0;
				const uint8_t* Plane = de265_get_image_plane(OutputImage, c, &Stride);

				int Width = de265_get_image_width(OutputImage, c);
				int Height = de265_get_image_height(OutputImage, c);
				assert(Stride > 0);

				for (int Row = 0; Row < Height; Row++)
				{
					const uint8_t* PosFrom = Plane + Row * Stride;
					memcpy(ImageBytes, PosFrom, Width);
					ImageBytes += Width;
				}
			}

			ImageBytes -= FrameSize;

			for (size_t i = 0; i < FrameSize; i++)
			{
				Image.Bytes.push_back(ImageBytes[i]);
			}

			Image.Size = FrameSize;
			Image.Config.BitDepth = 8;
			Image.Config.Width = FrameWidth;
			Image.Config.Height = FrameHeight;

			Image.Config.FramerateNum = 29997;
			Image.Config.FramerateDenom = 1000;

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new HevcResult(DE265_OK);
}

int HevcDecoder::ScaleX(int InX, EChromaFormat InFormat)
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

int HevcDecoder::ScaleY(int InY, EChromaFormat InFormat)
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

#undef LogHevcDecoder