
#pragma once

#include <string>

#include "ovb_send/packetizer/packetizer.h"

class HevcPacketizer : public Packetizer
{
public:
	HevcPacketizer();

	virtual void				   Packetize(std::vector<NALU> InNALUs) override;
	virtual std::vector<RTPPacket> Flush() override;

private:
	std::string NalTypeToString(uint8_t InNalType);
};