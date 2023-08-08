#pragma once

#include <memory>

#include "common.h"
#include "encoder.h"

class EncoderFactory
{
public:
	static std::shared_ptr<Encoder> Create(ECodec InCodec);
};