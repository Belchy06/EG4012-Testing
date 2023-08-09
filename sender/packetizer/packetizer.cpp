#include <assert.h>
#include <chrono>

#include "packetizer.h"

#ifndef RTP_PAYLOAD_SIZE
	#define RTP_PAYLOAD_SIZE 1200
#endif

std::shared_ptr<Packetizer> Packetizer::Self = nullptr;
uint16_t					Packetizer::SequenceNumber = 0;

std::shared_ptr<Packetizer> Packetizer::Create()
{
	if (Self == nullptr)
	{
		std::shared_ptr<Packetizer> Temp(new Packetizer);
		Self = Temp;
	}
	return Self;
}

std::vector<RTPPacket> Packetizer::Packetize(const uint8_t* InData, size_t InSize)
{
	assert(InSize > 0);

	std::vector<RTPPacket> Packets;

	size_t SizePacketized = 0;
	while (SizePacketized < InSize)
	{
		int PacketSize = std::min((size_t)RTP_PAYLOAD_SIZE, InSize - SizePacketized);
		// TODO (belchy06): RTP Timestamp
		Packets.push_back(RTPPacket(10, (int)SequenceNumber++, 0, InData + SizePacketized, PacketSize));
	}

	return Packets;
}