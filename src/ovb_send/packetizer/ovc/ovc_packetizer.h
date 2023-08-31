#pragma once

#include "ovb_send/packetizer/packetizer.h"

class OvcPacketizer : public Packetizer
{
public:
	OvcPacketizer();

	virtual void				   Packetize(std::vector<NALU> InNALUs) override;
	virtual std::vector<RTPPacket> Flush() override;
};