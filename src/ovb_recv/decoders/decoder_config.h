#pragma once

#include <ios>

#include "ovb_common/common.h"

class DecoderConfig
{
public:
	int			  Width;
	int			  Height;
	int			  FramerateNum;
	int			  FramerateDenom;
	int			  BitDepth;
	EChromaFormat Format;

	std::streamoff StartSkip;
	std::streamoff PictureSkip;

	ELogSeverity LogLevel;
};