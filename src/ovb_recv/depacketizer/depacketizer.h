#pragma once

#include <memory>
#include <vector>

#include "ovb_common/rtp/packet.h"
#include "ovb_recv/depacketizer/depacketizer_listener.h"

class Depacketizer
{
public:
	Depacketizer()
		: PrevTimestamp(0), PrevSequenceNumber(0)
	{
	}

	void RegiseterDepacketizerListener(IDepacketizerListener* InDepacketizerListener) { DepacketizerListener = InDepacketizerListener; }

	virtual void HandlePacket(RTPPacket InPacket) = 0;

protected:
	uint32_t			   PrevTimestamp;
	uint16_t			   PrevSequenceNumber;
	std::vector<RTPPacket> Fragments;

	IDepacketizerListener* DepacketizerListener;
};