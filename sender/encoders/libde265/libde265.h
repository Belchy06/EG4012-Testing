#pragma once

#include "encoder.h"

class Libde265Encoder : public Encoder
{
public:
	Libde265Encoder();
	~Libde265Encoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::istream* InStream) override;
};
