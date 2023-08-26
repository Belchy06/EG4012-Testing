#pragma once

#include "xvc_enc_lib/xvcenc.h"

class XvcEncoderConfig
{
public:
	int		 XvcNumRefPics;
	uint32_t XvcMaxKeypicDistance;
	int		 XvcQP;
};