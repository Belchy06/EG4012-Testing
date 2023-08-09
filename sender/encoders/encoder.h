#pragma once

#include <istream>
#include <fstream>

#include "common.h"
#include "encoder_callback.h"
#include "encoder_config.h"
#include "encoder_result.h"

// Encoder wrapper that wraps all of the third_party codecs
class Encoder
{
public:
	void RegisterEncodeCompleteCallback(IEncodeCompleteCallback* InEncoderCompleteCallback);

	virtual EncodeResult* Init(EncoderConfig& InConfig);
	virtual EncodeResult* Encode(std::istream* InStream);

protected:
	EncoderConfig			 Config;
	IEncodeCompleteCallback* OnEncodedImageCallback;
};