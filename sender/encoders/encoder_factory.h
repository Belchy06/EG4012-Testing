#pragma once

#include "common.h"
#include "encoder.h"

#include <memory>

class EncoderFactory
{
public:
	static std::shared_ptr<Encoder> Create(ECodec InCodec);
};