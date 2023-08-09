#pragma once

#include "common.h"
#include "decoder_callback.h"
#include "decoder_config.h"
#include "decoder_result.h"

// Decoder wrapper that wraps all of the third_party codecs
class Decoder
{
public:
	void RegisterDecodeCompleteCallback(IDecodeCompleteCallback* InDecoderCompleteCallback);

	virtual DecodeResult* Init(DecoderConfig& InConfig);
	virtual DecodeResult* Decode(const uint8_t* InNalBytes, size_t InNalSize);

protected:
	DecoderConfig			 Config;
	IDecodeCompleteCallback* OnDecodedImageCallback;
};