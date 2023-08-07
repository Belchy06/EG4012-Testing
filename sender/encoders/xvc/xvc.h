#pragma once

#include "encoder.h"

#include "xvc_enc_lib/xvcenc.h"

class XvcEncoder : public Encoder
{
public:
	XvcEncoder();
	~XvcEncoder();

	virtual EncodeResult Init(EncoderConfig& InConfig) override;
	virtual EncodeResult Encode() override;

private:
	const xvc_encoder_api*	Api;
	xvc_encoder_parameters* Params;
	xvc_encoder*			Encoder;
};