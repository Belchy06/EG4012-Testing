#pragma once

#include <ios>

#include "ovb_common/common.h"

class DecoderConfig
{
public:
	size_t		  Width;
	size_t		  Height;
	size_t		  FramerateNum;
	size_t		  FramerateDenom;
	size_t		  BitDepth;
	EChromaFormat Format;

	std::streamoff StartSkip;
	std::streamoff PictureSkip;

	ELogSeverity LogLevel;
};