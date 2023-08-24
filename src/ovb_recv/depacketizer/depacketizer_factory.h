#pragma once

#include <memory>

#include "ovb_common/common.h"
#include "ovb_recv/depacketizer/depacketizer.h"

class DepacketizerFactory
{
public:
	static std::shared_ptr<Depacketizer> Create(ECodec InCodec);
};