#pragma once

#include <memory>
#include <vector>

#include "rtp/packet.h"

class Packetizer
{
public:
	static std::shared_ptr<Packetizer> Create();
	std::vector<RTPPacket>			   Packetize(const uint8_t* InData, size_t InSize);

private:
	Packetizer() = default;

private:
	static std::shared_ptr<Packetizer> Self;

	static uint16_t SequenceNumber;
	static uint32_t LastTimestamp;
};