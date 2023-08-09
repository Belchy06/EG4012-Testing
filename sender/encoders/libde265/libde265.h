#pragma once

#include <vector>

#include "common.h"
#include "encoder.h"
#include "libde265/en265.h"

class Libde265Encoder : public Encoder
{
public:
	Libde265Encoder();
	~Libde265Encoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture) override;

private:
	de265_chroma GetChroma(EChromaFormat InFormat);
	int			 ScaleChroma(int InSize, EChromaFormat InFormat);

private:
	en265_encoder_context* Encoder;
};
