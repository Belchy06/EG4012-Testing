#include "decoder.h"

void Decoder::RegisterDecodeCompleteCallback(IDecodeCompleteCallback* InDecoderCompleteCallback)
{
	OnDecodedImageCallback = InDecoderCompleteCallback;
}

DecodeResult* Decoder::Init(DecoderConfig& InConfig)
{
	unimplemented();
	return new DecodeResult();
}

DecodeResult* Decoder::Decode(std::istream* InStream)
{
	unimplemented();
	return new DecodeResult();
}