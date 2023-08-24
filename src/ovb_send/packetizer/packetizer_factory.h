#pragma once

#include <memory>
#include "ovb_common/common.h"
#include "ovb_send/packetizer/packetizer.h"

class PacketizerFactory
{
public:
	static std::shared_ptr<Packetizer> Create(ECodec InCodec);
};