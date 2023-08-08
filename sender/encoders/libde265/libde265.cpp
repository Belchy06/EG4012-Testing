#include "libde265.h"
#include "libde265_result.h"
#include "libde265/de265.h"

#pragma comment(lib, "libde265.lib")

Libde265Encoder::Libde265Encoder()
{
}

Libde265Encoder::~Libde265Encoder()
{
}

EncodeResult* Libde265Encoder::Init(EncoderConfig& InConfig)
{
	return new Libde265Result(de265_init());
}

EncodeResult* Libde265Encoder::Encode(std::istream* InStream)
{
	return new EncodeResult();
}
