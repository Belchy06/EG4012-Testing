#pragma once

#include <istream>

#include "common.h"
#include "config.h"
#include "result.h"
#include "encoder_callback.h"

// Encoder wrapper that wraps all of the third_party codecs
class Encoder
{
public:
	virtual void RegisterEncodeCompleteCallback(IEncodeCompleteCallback* InEncoderCompleteCallback);

	virtual EncodeResult* Init(EncoderConfig& InConfig);
	virtual EncodeResult* Encode(std::istream* InStream);

protected:
	EncoderConfig			 Config;
	IEncodeCompleteCallback* OnEncodedImageCallback;
};