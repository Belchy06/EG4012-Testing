#pragma once

#include "ovb_send/packetizer/packetizer.h"

class XvcPacketizer : public Packetizer
{
public:
	XvcPacketizer();

	virtual void				   Packetize(std::vector<NALU> InNALUs) override;
	virtual std::vector<RTPPacket> Flush() override;
};