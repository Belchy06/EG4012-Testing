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
	de265_error Result = de265_init();
	if (Result != DE265_OK)
	{
		return new Libde265Result(Result);
	}

	Encoder = en265_new_encoder();

	de265_set_verbosity(3);

	Result = en265_start_encoder(Encoder, 0);
	return new Libde265Result(Result);
}

EncodeResult* Libde265Encoder::Encode(std::istream* InStream)
{
	return new EncodeResult();
}
