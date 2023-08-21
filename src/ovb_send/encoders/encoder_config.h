#pragma once

#include <ios>

#include "ovb_common/common.h"

class EncoderConfig
{
public:
	int			  Width;
	int			  Height;
	double		  Framerate;
	int			  BitDepth;
	EChromaFormat Format;

	std::streamoff StartSkip;
	std::streamoff PictureSkip;

	ELogSeverity LogLevel;
};