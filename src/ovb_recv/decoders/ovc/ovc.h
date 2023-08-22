#pragma once

#include "ovb_recv/decoders/decoder.h"

#include "ovc_dec/ovc_dec.h"
#include "ovc_dec/config.h"

class OvcDecoder : public Decoder
{
public:
	OvcDecoder();
	~OvcDecoder();

	virtual DecodeResult* Init(DecoderConfig& InConfig) override;
	virtual DecodeResult* Decode(uint8_t* InNalBytes, size_t InNalSize) override;

private:
	ovc_dec_config* Params;
	ovc_decoder*	Decoder;
};