#pragma once

#include "ovb_recv/depacketizer/depacketizer.h"

class HevcDepacketizer : public Depacketizer
{
public:
	virtual void HandlePacket(RTPPacket InPacket) override;
};