#pragma once

#include "packet.h"

class IRTPPacketListener
{
public:
	virtual void OnPacketReceived(RTPPacket InPacket) = 0;
};