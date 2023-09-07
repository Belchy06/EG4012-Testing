#pragma once

#include <vector>

#include "ovb_common/common.h"

class ImageConfig
{
public:
	size_t		  Width;
	size_t		  Height;
	size_t		  FramerateNum;
	size_t		  FramerateDenom;
	size_t		  BitDepth;
	EChromaFormat Format;
};

class DecodedImage
{
public:
	std::vector<uint8_t> Bytes;
	size_t				 Size;
	ImageConfig			 Config;
};