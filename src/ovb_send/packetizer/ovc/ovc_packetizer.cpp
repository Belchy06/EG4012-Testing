#include "ovb_common/common.h"

#include "ovb_send/packetizer/ovc/ovc_packetizer.h"

#define LogOvcPacketizer "LogOvcPacketizer"

std::vector<RTPPacket> OvcPacketizer::Packetize(uint8_t* InData, size_t InSize)
{
	// No RTP Spec is defined for OVC (We're writing it right here). For this case, we're going to base it off of
	// something similiar to HEVC's:
	// https://datatracker.ietf.org/doc/html/rfc7798#section-4.4.3
	assert(InSize > 0);

	std::vector<RTPPacket> Packets;

	// clang-format off
	uint8_t StartByte   = (InData[0] & 0b11111111) >> 0;
	uint8_t ZeroBits    = (InData[1] & 0b11000000) >> 6;
	uint8_t NalUnitType = (InData[1] & 0b00111111) >> 0;
	// clang-format on
	assert((StartByte & ZeroBits) == 0);

	LOG(LogOvcPacketizer, LOG_SEVERITY_DETAILS, "Packetizing NAL. Type: {}; Size: {}", +NalUnitType, InSize);

	if (InSize < RTP_PAYLOAD_SIZE)
	{
		// Single NAL
		// TODO (belchy06): Aggregation Packets
		RTPPacket Packet = RTPPacket(10, (size_t)SequenceNumber++, 0, InData, InSize, false);
		// TODO (belchy06): RTP Timestamp
		Packets.push_back(Packet);
	}
	else
	{
		// Update size to remove the two byte NAL header as we'll be re-writing this ourself
		InData += 2;
		InSize -= 2;
		/* The Structure of a Fragmentation Unit

		 +---------------+---------------+---------------+---------------+
		 |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
		 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 |    PayloadHdr (Type = 49)|    |   FU Header   | FU Payload ...|
		 +---------------+---------------+---------------+               +
		 |                              ...                              |
		 +---------------+---------------+---------------+---------------+
		*/

		// Fragmentation Unit
		std::vector<uint8_t> NALHeader;

		uint8_t NalByte;
		// clang-format off
		NalByte = 0;
		NALHeader.push_back(NalByte);
		// clang-format on

		// clang-format off
		NalByte = 0;
		NalByte |= (0           << 6) & 0b11000000;
		NalByte |= ((uint8_t)49 << 0) & 0b00111111;
		NALHeader.push_back(NalByte);
		// clang-format on

		size_t	SizePacketized = 0;
		uint8_t bIsFirst = 1;
		size_t	MaxSize = (size_t)RTP_PAYLOAD_SIZE - 3;
		while (SizePacketized < InSize)
		{
			size_t	PacketSize = std::min(MaxSize, InSize - SizePacketized);
			uint8_t bIsLast = PacketSize < MaxSize;

			/*
			The Structure of an FU Header

			 +---------------+
			 |0|1|2|3|4|5|6|7|
			 +-+-+-+-+-+-+-+-+
			 |S|E|     T     |
			 +---------------+

			S: 1 bit
				When set to 1, the S bit indicates the start of a fragmented NAL
				unit, i.e., the first byte of the FU payload is also the first
				byte of the payload of the fragmented NAL unit.  When the FU
				payload is not the start of the fragmented NAL unit payload, the S
				bit MUST be set to 0.

			E: 1 bit
				When set to 1, the E bit indicates the end of a fragmented NAL
				unit, i.e., the last byte of the payload is also the last byte of
				the fragmented NAL unit.  When the FU payload is not the last
				fragment of a fragmented NAL unit, the E bit MUST be set to 0.

			T: 6 bits
				The field T MUST be equal to the field Type of the fragmented
				NAL unit.
			*/
			std::vector<uint8_t> FUHeader;
			// clang-format off
			uint8_t FUHeaderByte = 0;
			FUHeaderByte |= (bIsFirst    << 7) & 0b10000000;
			FUHeaderByte |= (bIsLast     << 6) & 0b01000000;
			FUHeaderByte |= (NalUnitType << 0) & 0b00111111;
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