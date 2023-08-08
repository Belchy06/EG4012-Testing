#pragma once

#include "encoder.h"

#include "libde265/en265.h"

class Libde265Encoder : public Encoder
{
public:
	Libde265Encoder();
	~Libde265Encoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::istream* InStream) override;

private:
	en265_encoder_context* Encoder;
};
