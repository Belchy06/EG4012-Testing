#pragma once

#include <vector>

#include "encoder.h"
#include "bvc_enc/bvc_enc.h"

class BvcEncoder : public Encoder
{
public:
	BvcEncoder();
	~BvcEncoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture) override;

private:
	bvc_enc_config* Params;
	bvc_encoder*	Encoder;
};