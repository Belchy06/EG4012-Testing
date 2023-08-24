#pragma once

#include "ovb_send/packetizer/packetizer.h"

class XvcPacketizer : public Packetizer
{
public:
	std::vector<RTPPacket> Packetize(uint8_t* InData, size_t InSize) override;
};