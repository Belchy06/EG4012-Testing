#pragma once

#include "common.h"
#include "config.h"
#include "result.h"

// Encoder wrapper that wraps all of the third_party codecs
class Encoder
{
public:
	virtual EncodeResult* Init(EncoderConfig& InConfig)
	{
		unimplemented();
		return new EncodeResult();
	}

	virtual EncodeResult* Encode()
	{
		unimplemented();
		return new EncodeResult();
	}

private:
	EncoderConfig Config;
};