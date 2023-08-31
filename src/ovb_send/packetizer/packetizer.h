#pragma once

#include <memory>
#include <vector>

#include "ovb_common/rtp/packet.h"
#include "ovb_common/video/nal.h"

#ifndef RTP_PAYLOAD_SIZE
	#define RTP_PAYLOAD_SIZE 1200
#endif

class Packetizer
{
public:
	Packetizer() {}

	virtual void				   Packetize(std::vector<NALU> InNALUs) = 0;
	virtual std::vector<RTPPacket> Flush() = 0;

protected:
	uint16_t  SequenceNumber;
	uint32_t  LastTimestamp;
	RTPPacket AggregatedPacket;

	std::vector<RTPPacket> Packets;
};