#pragma once

#include "common.h"
#include "config.h"
#include "result.h"

// Encoder wrapper that wraps all of the third_party codecs
class Encoder
{
public:
	virtual EncodeResult Init(EncoderConfig& InConfig)
	{
		unimplemented();
		EncodeResult Res;
		return Res;
	}

	virtual EncodeResult Encode()
	{
		unimplemented();
		EncodeResult Res;
		return Res;
	}

private:
	EncoderConfig Config;
};