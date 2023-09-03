#pragma once

#include <vector>

#include "ovb_send/encoders/encoder.h"
#include "wels/codec_api.h"

class AvcEncoder : public Encoder
{
public:
	AvcEncoder();
	~AvcEncoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture) override;

private:
	SEncParamExt* Params;
	ISVCEncoder*  Encoder;
};