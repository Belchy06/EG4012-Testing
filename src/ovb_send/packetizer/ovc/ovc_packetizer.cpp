#include "ovb_common/common.h"

#include "ovb_send/packetizer/ovc/ovc_packetizer.h"

#define LogOvcPacketizer "LogOvcPacketizer"

OvcPacketizer::OvcPacketizer()
{
}

void OvcPacketizer::Packetize(std::vector<NALU> InNALUs)
{
	// No RTP Spec is defined for OVC (We're writing it right here). For this case, we're going to base it off of
	// something similiar to HEVC's:
	// https://datatracker.ietf.org/doc/html/rfc7798#section-4.4.3
	for (size_t i = 0; i < InNALUs.size(); i++)
	{
		NALU& Nal = InNALUs[i];

		assert(Nal.Size > 0);
		// clang-format off
	    uint8_t StartByte   = (Nal.Data[0] & 0b11111111) >> 0;
	    uint8_t ZeroBits    = (Nal.Data[1] & 0b11000000) >> 6;
	    uint8_t NalUnitType = (Nal.Data[1] & 0b00111111) >> 0;
		// clang-format on
		assert((StartByte & ZeroBits) == 0);

		LOG(LogOvcPacketizer, LOG_SEVERITY_DETAILS, "Packetizing NAL. Type: {}; Size: {}", +NalUnitType, Nal.Size);

		if (Nal.Size < RTP_PAYLOAD_SIZE)
		{
			if (AggregatedPacket.GetPayloadSize() > 0 && AggregatedPacket.GetPayloadSize() + Nal.Size > RTP_PAYLOAD_SIZE)
			{
				// This packet won't fit into the existing AP. Send the AP on its way
				Packets.push_back(RTPPacket(10, (size_t)SequenceNumber++, 0, AggregatedPacket.GetPayload(), AggregatedPacket.GetPayloadSize(), false));
				AggregatedPacket = RTPPacket();
			}

			// This NAL can fit into a single packet. Check if it should be aggregated first
			if (AggregatedPacket.GetPayloadSize() == 0 && (i + 1) < InNALUs.size() && Nal.Size + InNALUs[i + 1].Size < RTP_PAYLOAD_SIZE)
			{
				/* The Structure of an Aggregated Packet
				 +---------------+---------------+---------------+---------------+
				 |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
				 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				 |    PayloadHdr (Type = 48)     |          NAL 1 Size           |
				 +---------------+---------------+---------------+---------------+
				 |                       NAL 1 Data ...                          |
				 +---------------+---------------+-------------------------------+
				 |          NAL 2 Size           |          NAL 2 Data ...       |
				 +---------------+---------------+---------------+---------------+
				*/

				std::vector<uint8_t> RTPPayload;

				// The next packet will fit into an aggregated packet as well, start aggregating
				// This is the first packet in the AP.
				uint8_t APByte;
				// clang-format off
			    APByte = 0;
			    RTPPayload.push_back(APByte);
				// clang-format on

				// clang-format off
			    APByte = 0;
			    APByte |= (0           << 6) & 0b11000000;
			    //      AP Type
			    APByte |= (48 << 0) & 0b00111111;
			    RTPPayload.push_back(APByte);
				// clang-format on

				// NALU Size
				RTPPayload.push_back(((uint16_t)Nal.Size >> 8) & 0b11111111);
				RTPPayload.push_back(((uint16_t)Nal.Size >> 0) & 0b11111111);

				// NALU HDR + NALU Data
				for (size_t j = 0; j < Nal.Size; j++)
				{
					RTPPayload.push_back(Nal.Data[j]);
				}

				AggregatedPacket.SetPayload(RTPPayload.data(), RTPPayload.size());
			}
			else if (AggregatedPacket.GetPayloadSize() > 0 && AggregatedPacket.GetPayloadSize() + Nal.Size <= RTP_PAYLOAD_SIZE)
			{
				// Continue populating the aggregated packet
				/* The Structure of an Aggregated Packet
				 +---------------+---------------+---------------+---------------+
				 |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
				 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				 |    PayloadHdr (Type = 48)     |          NAL 1 Size           |
				 +---------------+---------------+---------------+---------------+
				 |                       NAL 1 Data ...                          |
				 +---------------+---------------+-------------------------------+
				 |          NAL 2 Size           |          NAL 2 Data ...       |
				 +---------------+---------------+---------------+---------------+
				*/
				std::vector<uint8_t> RTPPayload;

				uint8_t* APPayload = AggregatedPacket.GetPayload();
				for (size_t j = 0; j < AggregatedPacket.GetPayloadSize(); j++)
				{
					RTPPayload.push_back(APPayload[j]);
				}

				// NALU Size
				RTPPayload.push_back(((uint16_t)Nal.Size >> 8) & 0b11111111);
				RTPPayload.push_back(((uint16_t)Nal.Size >> 0) & 0b11111111);

				// NALU HDR + NALU Data
				for (size_t j = 0; j < Nal.Size; j++)
				{
					RTPPayload.push_back(Nal.Data[j]);
				}

				AggregatedPacket.SetPayload(RTPPayload.data(), RTPPayload.size());
			}
			else
			{
				// Single NAL packet
				if (AggregatedPacket.GetPayloadSize() > 0)
				{
					Packets.push_back(RTPPacket(10, (size_t)SequenceNumber++, 0, AggregatedPacket.GetPayload(), AggregatedPacket.GetPayloadSize(), false));
					AggregatedPacket = RTPPacket();
				}

				// Single NAL
				RTPPacket Packet = RTPPacket(10, (size_t)SequenceNumber++, 0, Nal.Data, Nal.Size, false);
				// TODO (belchy06): RTP Timestamp
				Packets.push_back(Packet);
			}
		}
		else
		{
			if (AggregatedPacket.GetPayloadSize() > 0)
			{
				Packets.push_back(RTPPacket(10, (size_t)SequenceNumber++, 0, AggregatedPacket.GetPayload(), AggregatedPacket.GetPayloadSize(), false));
				AggregatedPacket = RTPPacket();
			}

			// Update size to remove the two byte NAL header as we'll be re-writing this ourself
			Nal.Data += 2;
			Nal.Size -= 2;
			/* The Structure of a Fragmentation Unit

			 +---------------+---------------+---------------+---------------+
			 |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
			 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 |    PayloadHdr (Type = 49)     |   FU Header   | FU Payload ...|
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
			while (SizePacketized < Nal.Size)
			{
				size_t	PacketSize = std::min(MaxSize, Nal.Size - SizePacketized);
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
				for (size_t j = 0; j < PacketSize; j++)
				{
					FUPayload.push_back((Nal.Data + SizePacketized)[j]);
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
	}
}

std::vector<RTPPacket> OvcPacketizer::Flush()
{
	if (AggregatedPacket.GetPayloadSize() > 0)
	{
		Packets.push_back(RTPPacket(10, (size_t)SequenceNumber++, 0, AggregatedPacket.GetPayload(), AggregatedPacket.GetPayloadSize(), false));
		AggregatedPacket = RTPPacket();
	}

	std::vector<RTPPacket> ReturnPackets = Packets;
	Packets.clear();
	return ReturnPackets;
};

#undef LogOvcPacketizer