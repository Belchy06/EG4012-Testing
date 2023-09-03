#pragma once

#include "ovb_recv/decoders/decoder.h"

#include "xvc_dec_lib/xvcdec.h"

class XvcDecoder : public Decoder
{
public:
	XvcDecoder();
	~XvcDecoder();

	virtual DecodeResult* Init(DecoderConfig& InConfig) override;
	virtual DecodeResult* Decode(uint8_t* InNalBytes, size_t InNalSize) override;

private:
	int ScaleX(int InX, EChromaFormat InFormat);
	int ScaleY(int InY, EChromaFormat InFormat);

private:
	const xvc_decoder_api*	Api;
	xvc_decoder_parameters* Params;
	xvc_decoder*			Decoder;
};