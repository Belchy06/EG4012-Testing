#pragma once

#include "encoder.h"

#include "xvc_enc_lib/xvcenc.h"

class XVCEncoder : public Encoder
{
	virtual void Encode() override;
};