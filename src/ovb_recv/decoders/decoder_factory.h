#pragma once

#include <memory>

#include "ovb_common/common.h"
#include "decoder.h"

class DecoderFactory
{
public:
	static std::shared_ptr<Decoder> Create(ECodec InCodec);
};