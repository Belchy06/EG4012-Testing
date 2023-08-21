#pragma once

#include <vector>

#include "ovb_send/encoders/encoder.h"
#include "vvenc/vvenc.h"
#include "ovb_send/encoders/vvc/vvc_result.h"

class VvcEncoder : public Encoder
{
public:
	VvcEncoder();
	~VvcEncoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture) override;

private:
	int	   ScaleX(int InX, EChromaFormat InFormat);
	int	   ScaleY(int InY, EChromaFormat InFormat);

private:
	vvenc_config* Params;
	vvencEncoder* Encoder;

	int64_t SequenceNumber;
};