#pragma once

#include "common.h"
#include "encoder.h"

class EncoderFactory
{
public:
	static Encoder* Create(ECodec InCodec);
};