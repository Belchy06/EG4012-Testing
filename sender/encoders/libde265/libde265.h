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
	virtual EncodeResult* Encode(std::istream* InStream) override;

private:
	bool		 ReadNextPicture(std::istream* InStream, std::vector<uint8_t>& OutPictureBytes);
	de265_chroma GetChroma(EChromaFormat InFormat);
	int			 ScaleChroma(int InSize, EChromaFormat InFormat);

private:
	en265_encoder_context* Encoder;
	std::vector<uint8_t>   PictureBytes;
};
