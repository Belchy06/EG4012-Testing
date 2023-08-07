#pragma once

#include <fstream>

#include "common.h"

struct SPictureFormat
{
	int			  Width = 0;
	int			  Height = 0;
	double		  Framerate = 0;
	int			  BitDepth = 0;
	EChromaFormat Format = EChromaFormat::CHROMA_FORMAT_UNDEFINED;
};

class CY4mReader
{
public:
	CY4mReader(std::istream* InStream);
	bool Read(SPictureFormat& OutFormat, std::streamoff& OutStartSkip, std::streamoff& OutPictureSkip);

private:
	std::istream* Stream;
};