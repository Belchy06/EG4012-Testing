#pragma once

#include <vector>

#include "ovb_send/encoders/encoder.h"
#include "libde265/en265.h"

class HevcEncoder : public Encoder
{
public:
	HevcEncoder();
	~HevcEncoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture) override;

private:
	en265_encoder_context* Encoder;
};