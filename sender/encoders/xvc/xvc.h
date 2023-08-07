#pragma once

#include "encoder.h"

#include "xvc_enc_lib/xvcenc.h"

class XVCEncoder : public Encoder
{
	XVCEncoder();
	~XVCEncoder();

	virtual void Init(Config& InConfig) override;
	virtual void Encode() override;

private:
	const xvc_encoder_api*	Api;
	xvc_encoder_parameters* Params;
};