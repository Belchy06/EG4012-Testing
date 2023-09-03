#pragma once

#include <iostream>
#include <sstream>
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
	int ReadNalFromStream(std::stringstream* InStream, vvencAccessUnit* OutAccessUnit);
	int RetrieveNalStartCode(unsigned char* pB, int InZerosInStartcode);

private:
	vvenc_config* Params;
	vvencEncoder* Encoder;

	int64_t SequenceNumber;
};