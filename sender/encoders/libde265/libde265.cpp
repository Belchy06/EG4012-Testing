#include "libde265.h"

Libde265Encoder::Libde265Encoder()
{
}

Libde265Encoder::~Libde265Encoder()
{
}

EncodeResult* Libde265Encoder::Init(EncoderConfig& InConfig)
{
	return new EncodeResult();
}

EncodeResult* Libde265Encoder::Encode(std::istream* InStream)
{
	return new EncodeResult();
}
