#pragma once

#include <fstream>

#include "ovb_common/common.h"

struct PictureFormat
{
	int			  Width = 0;
	int			  Height = 0;
	double		  Framerate = 0;
	int			  BitDepth = 0;
	EChromaFormat Format = EChromaFormat::CHROMA_FORMAT_UNDEFINED;
};

class Y4mReader
{
public:
	Y4mReader(std::istream* InStream);
	bool Read(PictureFormat& OutFormat, std::streamoff& OutPictureSkip);

private:
	std::istream* Stream;
};