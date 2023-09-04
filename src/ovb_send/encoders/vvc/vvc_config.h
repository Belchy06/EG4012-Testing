#pragma once

#include "vvenc/vvenc.h"

class VvcEncoderConfig
{
public:
	int VvcGOPSize;
	int VvcIntraPeriod;
	int VvcQP;
};