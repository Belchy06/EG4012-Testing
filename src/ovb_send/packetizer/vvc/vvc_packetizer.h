#pragma once

#include "ovb_send/packetizer/packetizer.h"

class VvcPacketizer : public Packetizer
{
public:
	VvcPacketizer();

	virtual void				   Packetize(std::vector<NALU> InNALUs) override;
	virtual std::vector<RTPPacket> Flush() override;
};