#pragma once

#include "common.h"
#include "encoder.h"

class EncoderFactory
{
	static Encoder* Create(ECodec InCodec);
}