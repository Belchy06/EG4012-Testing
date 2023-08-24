#pragma once

#include <memory>
#include <vector>

#include "ovb_common/rtp/packet.h"

#ifndef RTP_PAYLOAD_SIZE
	#define RTP_PAYLOAD_SIZE 1200
#endif

class Packetizer
{
public:
	virtual std::vector<RTPPacket> Packetize(uint8_t* InData, size_t InSize) = 0;

protected:
	uint16_t SequenceNumber;
	uint32_t LastTimestamp;
};