#pragma once

#include "decoder.h"

#include "bvc_dec/bvc_dec.h"
#include "bvc_dec/config.h"

class BvcDecoder : public Decoder
{
public:
	BvcDecoder();
	~BvcDecoder();

	virtual DecodeResult* Init(DecoderConfig& InConfig) override;
	virtual DecodeResult* Decode(const uint8_t* InNalBytes, size_t InNalSize) override;

private:
	bvc_dec_config* Params;
	bvc_decoder*	Decoder;
};