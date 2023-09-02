#pragma once

#include "ovb_recv/decoders/decoder.h"

#include "wels/codec_api.h"

class AvcDecoder : public Decoder
{
public:
	AvcDecoder();
	~AvcDecoder();

	virtual DecodeResult* Init(DecoderConfig& InConfig) override;
	virtual DecodeResult* Decode(uint8_t* InNalBytes, size_t InNalSize) override;

private:
	int ScaleX(int InX, EChromaFormat InFormat);
	int ScaleY(int InY, EChromaFormat InFormat);

private:
	SDecodingParam* Params;
	ISVCDecoder*	Decoder;
};