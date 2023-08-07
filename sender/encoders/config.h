#pragma once

#include "common.h"

class EncoderConfig
{
public:
	int			  Width;
	int			  Height;
	double		  Framerate;
	int			  BitDepth;
	EChromaFormat Format;
};