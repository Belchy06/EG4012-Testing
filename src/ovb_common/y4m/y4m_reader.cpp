#include <assert.h>
#include <iostream>
#include <string>

#include "y4m_reader.h"

Y4mReader::Y4mReader(std::istream* InStream)
	: Stream(InStream)
{
}

bool Y4mReader::Read(PictureFormat& OutFormat, std::streamoff& OutPictureSkip)
{
	PictureFormat Picture;
	char		  Buf[80];

	std::streamsize Len;
	for (Len = 0; Len < sizeof(Buf) - 1; Len++)
	{
		int Ret = Stream->get();
		if (Ret == std::istream::traits_type::eof())
		{
			return false;
		}
		Buf[Len] = static_cast<char>(Ret);
		if (Buf[Len] == '\n')
		{
			break;
		}
	}
	Buf[Len] = 0;

	std::streamoff Pos = 10;
	int			   Den, Num;
	char*		   End;
	while (Pos < Len)
	{
		if (Buf[Pos] == '\n')
		{
			break;
		}
		if (Buf[Pos] == ' ')
		{
			Pos++;
			continue;
		}
		switch (Buf[Pos++])
		{
			case 'W': // picture width
				Picture.Width = static_cast<int>(strtol(Buf + Pos, &End, 10));
				Pos = End - Buf;
				break;
			case 'H': // picture height
				Picture.Height = static_cast<int>(strtol(Buf + Pos, &End, 10));
				Pos = End - Buf;
				break;
			case 'F': // framerate
				Den = static_cast<int>(strtol(Buf + Pos, &End, 10));
				Pos = End - Buf + 1;
				Num = static_cast<int>(strtol(Buf + Pos, &End, 10));
				Pos = End - Buf;
				Picture.Framerate = static_cast<double>(Den) / Num;
				break;
			case 'I': // interlacing
				assert(Buf[Pos] == 'p');
				Pos++;
				break;
			case 'A': // sample aspect ratio
				strtol(Buf + Pos, &End, 10);
				Pos = End - Buf + 1;
				strtol(Buf + Pos, &End, 10);
				Pos = End - Buf;
				break;
			case 'C': // color space
				if (!strncmp(Buf + Pos, "420p12", 6))
				{
					Picture.BitDepth = 12;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_420;
					Pos += 6;
				}
				else if (!strncmp(Buf + Pos, "420p10", 6))
				{
					Picture.BitDepth = 10;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_420;
					Pos += 6;
				}
				else if (!strncmp(Buf + Pos, "420", 3))
				{
					Picture.BitDepth = 8;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_420;
					Pos += 3;
				}
				else if (!strncmp(Buf + Pos, "422p12", 6))
				{
					Picture.BitDepth = 12;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_422;
					Pos += 6;
				}
				else if (!strncmp(Buf + Pos, "422p10", 6))
				{
					Picture.BitDepth = 10;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_422;
					Pos += 6;
				}
				else if (!strncmp(Buf + Pos, "422", 3))
				{
					Picture.BitDepth = 8;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_422;
					Pos += 3;
				}
				else if (!strncmp(Buf + Pos, "444p12", 6))
				{
					Picture.BitDepth = 12;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_444;
					Pos += 6;
				}
				else if (!strncmp(Buf + Pos, "444p10", 6))
				{
					Picture.BitDepth = 10;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_444;
					Pos += 6;
				}
				else if (!strncmp(Buf + Pos, "444", 3))
				{
					Picture.BitDepth = 8;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_444;
					Pos += 3;
				}
				else if (!strncmp(Buf + Pos, "mono", 4))
				{
					Picture.BitDepth = 8;
					Picture.Format = EChromaFormat::CHROMA_FORMAT_400;
					Pos += 4;
				}
				else
				{
					Picture.Format = EChromaFormat::CHROMA_FORMAT_UNDEFINED;
				}
				break;
			case 'X': // YUV4MPEG2 comment, ignored
			default:
				while (Pos < Len && Buf[Pos] != ' ' && Buf[Pos] != '\n')
				{
					Pos++;
				}
				break;
		}
	}

	assert(Picture.Width != 0 && Picture.Height != 0);
	assert(Picture.BitDepth != 0);
	assert(Picture.Format != EChromaFormat::CHROMA_FORMAT_UNDEFINED);

	OutFormat = Picture;
	OutPictureSkip = 6; // Skip "FRAME\n" before each picture

	return true;
}