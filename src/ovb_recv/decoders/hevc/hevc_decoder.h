#pragma once

#include "ovb_recv/decoders/decoder.h"

#include "libde265/de265.h"

class HevcDecoder : public Decoder
{
public:
	HevcDecoder();
	~HevcDecoder();

	virtual DecodeResult* Init(DecoderConfig& InConfig) override;
	virtual DecodeResult* Decode(uint8_t* InNalBytes, size_t InNalSize) override;

private:
	int ScaleX(int InX, EChromaFormat InFormat);
	int ScaleY(int InY, EChromaFormat InFormat);

private:
	de265_decoder_context* Decoder;
};