
#pragma once

#include "ovb_send/packetizer/packetizer.h"

class AvcPacketizer : public Packetizer
{
public:
	AvcPacketizer();

	virtual void				   Packetize(std::vector<NALU> InNALUs) override;
	virtual std::vector<RTPPacket> Flush() override;
};