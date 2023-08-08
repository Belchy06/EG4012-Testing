#include <algorithm>

#include "libde265.h"
#include "libde265_result.h"
#include "libde265/de265.h"
#include "libde265/image.h"

#pragma comment(lib, "libde265.lib")

Libde265Encoder::Libde265Encoder()
{
}

Libde265Encoder::~Libde265Encoder()
{
	if (Encoder)
	{
		en265_free_encoder(Encoder);
	}

	de265_free();
}

EncodeResult* Libde265Encoder::Init(EncoderConfig& InConfig)
{
	Config = InConfig;

	de265_error Result = de265_init();
	if (Result != DE265_OK)
	{
		return new Libde265Result(Result);
	}

	Encoder = en265_new_encoder();

	de265_set_verbosity(static_cast<int>(Config.LogLevel));

	std::string array[] = {
		"-sop-structure",
		"intra"
	};
	std::vector<char*> vec;
	std::transform(std::begin(array), std::end(array),
		std::back_inserter(vec),
		[](std::string& s) { s.push_back(0); return &s[0]; });
	vec.push_back(nullptr);
	int temp = vec.size();
	Result = en265_parse_command_line_parameters(Encoder, &temp, vec.data());
	if (Result != DE265_OK)
	{
		return new Libde265Result(Result);
	}

	Result = en265_start_encoder(Encoder, 0);
	return new Libde265Result(Result);
}

EncodeResult* Libde265Encoder::Encode(std::istream* InStream)
{
	int PictureSamples = 0;
	if (Config.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME)
	{
		PictureSamples = Config.Width * Config.Height;
	}
	else if (Config.Format == EChromaFormat::CHROMA_FORMAT_420)
	{
		PictureSamples = (3 * (Config.Width * Config.Height)) >> 1;
	}
	else if (Config.Format == EChromaFormat::CHROMA_FORMAT_422)
	{
		PictureSamples = 2 * Config.Width * Config.Height;
	}
	else if (Config.Format == EChromaFormat::CHROMA_FORMAT_444)
	{
		PictureSamples = 3 * Config.Width * Config.Height;
	}

	PictureBytes.resize(Config.BitDepth == 8 ? PictureSamples : (PictureSamples << 1));

	de265_error Result;
	bool		bContinue = true;
	while (bContinue)
	{
		if (!ReadNextPicture(InStream, PictureBytes))
		{
			en265_push_eof(Encoder);
			bContinue = false;
		}
		else
		{
			de265_image* Input = new de265_image;
			Result = Input->alloc_image(Config.Width, Config.Height, GetChroma(Config.Format), NULL, false, NULL, 0, NULL, false);
			if (Result != DE265_OK)
			{
				return new Libde265Result(Result);
			}

			int		 NumComponents = Config.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME ? 1 : 3;
			uint8_t* SrcBytes = PictureBytes.data();
			int		 FrameSize = Config.Width * Config.Height;
			for (int c = 0; c < NumComponents; c++)
			{
				const EYuvComponent Component = static_cast<EYuvComponent>(c);
				uint8_t*			Plane = Input->get_image_plane(c);
				int					Stride = Input->get_image_stride(c);
				if (Component == EYuvComponent::Y)
				{
					memcpy(Plane, SrcBytes, FrameSize);
				}
				else if (Component == EYuvComponent::U)
				{
					switch (Config.Format)
					{
						case EChromaFormat::CHROMA_FORMAT_420:
							memcpy(Plane, SrcBytes + FrameSize, (FrameSize) / 4);
							break;
						case EChromaFormat::CHROMA_FORMAT_422:
							memcpy(Plane, SrcBytes + FrameSize, (Config.Width / 2) * Config.Height);
							break;
						case EChromaFormat::CHROMA_FORMAT_444:
							memcpy(Plane, SrcBytes + FrameSize, FrameSize);
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
							memcpy(Plane, SrcBytes + (5 / 4) * FrameSize, (FrameSize) / 4);
							break;
						case EChromaFormat::CHROMA_FORMAT_422:
							memcpy(Plane, SrcBytes + (Config.Width * Config.Width) + (3 * Config.Width * Config.Height) + (2 * Config.Height * Config.Height), (Config.Width / 2) * Config.Height);
							break;
						case EChromaFormat::CHROMA_FORMAT_444:
							memcpy(Plane, SrcBytes + 2 * FrameSize, FrameSize);
							break;
						case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
						case EChromaFormat::CHROMA_FORMAT_UNDEFINED:
						default:
							break;
					}
				}
			}

			Result = en265_push_image(Encoder, Input);
			if (Result != DE265_OK)
			{
				return new Libde265Result(Result);
			}
		}

		Result = en265_encode(Encoder);
		if (Result != DE265_OK)
		{
			return new Libde265Result(Result);
		}

		for (;;)
		{
			en265_packet* Packet = en265_get_packet(Encoder, 0);
			if (Packet == nullptr)
			{
				break;
			}

			OnEncodedImageCallback->OnEncodeComplete(Packet->data, Packet->length);

			en265_free_packet(Encoder, Packet);
		}
	}

	return new Libde265Result(DE265_OK);
}

bool Libde265Encoder::ReadNextPicture(std::istream* InStream, std::vector<uint8_t>& OutPictureBytes)
{
	InStream->read(reinterpret_cast<char*>(&(OutPictureBytes)[0]), OutPictureBytes.size());
	return InStream->gcount() == static_cast<int>(OutPictureBytes.size());
}

de265_chroma Libde265Encoder::GetChroma(EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
			return de265_chroma_mono;
		case EChromaFormat::CHROMA_FORMAT_420:
			return de265_chroma_420;
		case EChromaFormat::CHROMA_FORMAT_422:
			return de265_chroma_422;
		case EChromaFormat::CHROMA_FORMAT_444:
			return de265_chroma_444;
		case EChromaFormat::CHROMA_FORMAT_UNDEFINED:
		default:
			return de265_chroma_mono;
	}
}

int Libde265Encoder::ScaleChroma(int InSize, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
			return 0;
		case EChromaFormat::CHROMA_FORMAT_420:
			return InSize >> 1;
		case EChromaFormat::CHROMA_FORMAT_422:
			return InSize >> 1;
		case EChromaFormat::CHROMA_FORMAT_444:
			return InSize;
		case EChromaFormat::CHROMA_FORMAT_UNDEFINED:
		default:
			assert(0);
			return 0;
	}
}