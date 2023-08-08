#include "encoder.h"

void Encoder::RegisterEncodeCompleteCallback(IEncodeCompleteCallback* InEncoderCompleteCallback)
{
	OnEncodedImageCallback = InEncoderCompleteCallback;
}

EncodeResult* Encoder::Init(EncoderConfig& InConfig)
{
	unimplemented();
	return new EncodeResult();
}

EncodeResult* Encoder::Encode(std::istream* InStream)
{
	unimplemented();
	return new EncodeResult();
}