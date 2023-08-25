#pragma once

#include "ovc_enc/config.h"

class OvcEncoderConfig
{
public:
	float OvcBitsPerPixel;
	bool  OvcRepeatVPS;

	int OvcNumStreamsExp = -1;
	int OvcNumLevels = -1;

	ovc_wavelet_family OvcWaveletFamily;
	ovc_wavelet_config OvcWaveletConfig;
	ovc_partition	   OvcPartitionType;
	ovc_spiht		   OvcSPIHT;
	ovc_entropy_coder  OvcEntropyCoder;
};