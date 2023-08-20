#pragma once

#include <vector>

#include "encoder.h"
#include "ovc_enc/ovc_enc.h"

class OvcEncoder : public Encoder
{
public:
	OvcEncoder();
	~OvcEncoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture) override;

private:
	ovc_enc_config* Params;
	ovc_encoder*	Encoder;
};