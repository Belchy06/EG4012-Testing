#pragma once

#include "vvenc/vvenc.h"

class VvcEncoderConfig
{
public:
	int VvcGOPSize = 0;
	int VvcIntraPeriod = 0;
	int VvcQP = -1;
	int VvcBitrate = -1;
};