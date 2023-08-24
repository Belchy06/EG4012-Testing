#include <assert.h>
#include <iostream>

#include "ovb_common/common.h"
#include "ovb_send/packetizer/vvc/vvc_packetizer.h"

std::vector<RTPPacket> VvcPacketizer::Packetize(uint8_t* InData, size_t InSize)
{
	assert(InSize > 0);

	if (InData[2] == 1)
	{
		InData += 3;
		InSize -= 3;
	}
	else if (InData[3] == 1)
	{
		InData += 4;
		InSize -= 4;
	}
	else
	{
		// TODO (belchy06): Handle gracefully
		unimplemented();
	}

	std::vector<RTPPacket> Packets;

	// clang-format off
	uint8_t ForbiddenZeroBit =   (InData[0] & 0b10000000) >> 7;
	uint8_t NuhReservedZeroBit = (InData[0] & 0b01000000) >> 6;
	uint8_t NuhLayerId =         (InData[0] & 0b00111111) >> 0;
	uint8_t NalUnitType =        (InData[1] & 0b11111000) >> 3;
	uint8_t NuhTemporalIdPlus1 = (InData[1] & 0b00000111) >> 0;
	// clang-format on
	assert(ForbiddenZeroBit == 0);
	assert(NuhReservedZeroBit == 0);
	std::cout << "NuhLayerId: " << +NuhLayerId << std::endl;
	std::cout << "NalUnitType: " << +NalUnitType << std::endl;
	std::cout << "NuhTemporalIdPlus1: " << +NuhTemporalIdPlus1 << std::endl;

	if (InSize < RTP_PAYLOAD_SIZE)
	{
		// Single NAL (4.3.1 https://www.rfc-editor.org/rfc/rfc9328.pdf)
		// TODO (belchy06): Aggregation Packets

		// TODO (belchy06): Removing the 0x0 0x0 0x1 or 0x0 0x0 0x0 0x1 may cause decoder issues. Check
		RTPPacket Packet = RTPPacket(10, (size_t)SequenceNumber++, 0, InData, InSize, false);
		// TODO (belchy06): RTP Timestamp
		Packets.push_back(Packet);
	}
	else
	{
		// Update size to remove the two byte NAL header as we'll be re-writing this ourself
		InData += 2;
		InSize -= 2;
		// Fragmentation Unit (4.3.3 https://www.rfc-editor.org/rfc/rfc9328.pdf)
		std::vector<uint8_t> NALHeader;
		uint8_t				 NalByte;
		// clang-format off
        NalByte = 0;
		NalByte |= (ForbiddenZeroBit   << 7) & 0b10000000;
		NalByte |= (NuhReservedZeroBit << 6) & 0b01000000;
		NalByte |= (NuhLayerId         << 0) & 0b00111111;
        NALHeader.push_back(NalByte);
		// clang-format on

		// clang-format off
        NalByte = 0;
        //             FU Type
		NalByte |= ((uint8_t)29        << 3) & 0b11111000;
		NalByte |= (NuhTemporalIdPlus1 << 0) & 0b00000111;
        NALHeader.push_back(NalByte);
		// clang-format on

		size_t	SizePacketized = 0;
		uint8_t bIsFirst = 1;
		size_t	MaxSize = (size_t)RTP_PAYLOAD_SIZE - 3;
		while (SizePacketized < InSize)
		{
			size_t	PacketSize = std::min(MaxSize, InSize - SizePacketized);
			uint8_t bIsLast = PacketSize < MaxSize;

			std::vector<uint8_t> FUHeader;
			// clang-format off
			uint8_t FUHeaderByte = 0;
            FUHeaderByte |= (bIsFirst    << 7) & 0b10000000;
            FUHeaderByte |= (bIsLast     << 6) & 0b01000000;
            // TODO (belch06): P bit ( indicates the last FU of the last VCL NAL unit of a coded picture )
            FUHeaderByte |= (0           << 5) & 0b00100000;
            FUHeaderByte |= (NalUnitType << 0) & 0b00011111;
            FUHeader.push_back(FUHeaderByte);
			// clang-format on

			std::vector<uint8_t> FUPayload;
			for (size_t i = 0; i < PacketSize; i++)
			{
				FUPayload.push_back((InData + SizePacketized)[i]);
			}

			std::vector<uint8_t> RTPPayload;
			RTPPayload.reserve(NALHeader.size() + FUHeader.size() + FUPayload.size());
			RTPPayload.insert(RTPPayload.end(), NALHeader.begin(), NALHeader.end());
			RTPPayload.insert(RTPPayload.end(), FUHeader.begin(), FUHeader.end());
			RTPPayload.insert(RTPPayload.end(), FUPayload.begin(), FUPayload.end());

			RTPPacket Packet = RTPPacket(10, (size_t)SequenceNumber++, 0, RTPPayload.data(), RTPPayload.size(), false);
			// TODO (belchy06): RTP Timestamp
			Packets.push_back(Packet);

			SizePacketized += PacketSize;
			bIsFirst = 0;
		}
	}

	return Packets;
}