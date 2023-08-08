#include "libde265.h"
#include "libde265_result.h"

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
