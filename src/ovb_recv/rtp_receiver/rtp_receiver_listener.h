#pragma once

#include "ovb_common/rtp/packet.h"

class IRTPPacketListener
{
public:
	virtual void OnPacketReceived(RTPPacket InPacket) = 0;
};