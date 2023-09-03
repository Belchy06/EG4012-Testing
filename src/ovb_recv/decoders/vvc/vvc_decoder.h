#pragma once

#include "ovb_recv/decoders/decoder.h"

#include "vvdec/vvdec.h"

class VvcDecoder : public Decoder
{
public:
	VvcDecoder();
	~VvcDecoder();

	virtual DecodeResult* Init(DecoderConfig& InConfig) override;
	virtual DecodeResult* Decode(uint8_t* InNalBytes, size_t InNalSize) override;

private:
	std::string GetNalUnitTypeAsString(vvdecNalType InNalType);
	int			ScaleX(int InX, EChromaFormat InFormat);
	int			ScaleY(int InY, EChromaFormat InFormat);

private:
	vvdecAccessUnit* AccessUnit;

	vvdecParams*  Params;
	vvdecDecoder* Decoder;
};