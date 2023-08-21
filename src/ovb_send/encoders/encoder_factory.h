#pragma once

#include <memory>

#include "ovb_common/common.h"
#include "encoder.h"

class EncoderFactory
{
public:
	static std::shared_ptr<Encoder> Create(ECodec InCodec);
};