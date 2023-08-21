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

DecodeResult* Decoder::Decode(const uint8_t* InNalBytes, size_t InNalSize)
{
	unimplemented();
	return new DecodeResult();
}