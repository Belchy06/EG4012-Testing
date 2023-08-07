#pragma once

#include "common.h"
#include "config.h"

// Encoder wrapper that wraps all of the third_party codecs
class Encoder
{
public:
	virtual void Init(Config& InConfig) { unimplemented(); }
	virtual void Encode() { unimplemented(); }

private:
	Config EncoderConfig;
};