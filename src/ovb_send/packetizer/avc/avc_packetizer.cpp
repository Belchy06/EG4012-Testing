
#include "ovb_common/common.h"

#include "ovb_send/packetizer/avc/avc_packetizer.h"

#define LogAvcPacketizer "LogAvcPacketizer"

AvcPacketizer::AvcPacketizer()
{
}

void AvcPacketizer::Packetize(std::vector<NALU> InNALUs)
{
	// https://datatracker.ietf.org/doc/html/rfc6184
	for (size_t i = 0; i < InNALUs.size(); i++)
	{
		NALU& Nal = InNALUs[i];

		assert(Nal.Size > 0);
		// clang-format off
		uint8_t StartByte   = (Nal.Data[0] & 0b11111111) >> 0;
		uint8_t FBit        = (StartByte   & 0b10000000) >> 7;
		uint8_t NRI         = (StartByte   & 0b01100000) >> 5;
		uint8_t NalUnitType = (StartByte   & 0b00011111) >> 0;
		// clang-format on
		assert(FBit == 0);

		LOG(LogAvcPacketizer, LOG_SEVERITY_DETAILS, "Packetizing NAL. Type: {}; Size: {}", +NalUnitType, Nal.Size);

		if (Nal.Size < RTP_PAYLOAD_SIZE)
		{
			if (AggregatedPacket.GetPayloadSize() > 0 && AggregatedPacket.GetPayloadSize() + Nal.Size > RTP_PAYLOAD_SIZE)
			{
				// This packet won't fit into the existing AP. Send the AP on its way
				Packets.push_back(RTPPacket(10, (size_t)SequenceNumber++, 0, AggregatedPacket.GetPayload(), AggregatedPacket.GetPayloadSize(), false));
				AggregatedPacket = RTPPacket();
			}

			// // This NAL can fit into a single packet. Check if it should be aggregated first
			if (AggregatedPacket.GetPayloadSize() == 0 && (i + 1) < InNALUs.size() && Nal.Size + InNALUs[i + 1].Size < RTP_PAYLOAD_SIZE)
			{
				/* The Structure of an Aggregated Packet
				 +---------------+---------------+---------------+---------------+
				 |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
				 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				 |Header(Type=24)|          NAL 1 Size           | NAL 1 Data... |
				 +---------------+---------------+---------------+---------------+
				 |                              ...                              |
				 +---------------+---------------+-------------------------------+
				 |          NAL 2 Size           |          NAL 2 Data ...       |
				 +---------------+---------------+---------------+---------------+
				*/

				std::vector<uint8_t> RTPPayload;

				// The next packet will fit into an aggregated packet as well, start aggregating
				// This is the first packet in the AP.
				uint8_t APByte;
				// clang-format off
                // AP Type
                // Only STAP-A supported from Table 4
			    APByte = 0;
			    APByte |= (0    << 7) & 0b10000000;
                APByte |= (NRI  << 5) & 0b01100000;
			    APByte |= (24   << 0) & 0b00011111;
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
				 |Header(Type=24)|          NAL 1 Size           | NAL 1 Data... |
				 +---------------+---------------+---------------+---------------+
				 |                              ...                              |
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

			/* The Structure of a Fragmentation Unit

			 +---------------+---------------+---------------+---------------+
			 |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
			 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 |Header(Type=28)|   FU Header   |          FU Payload ...       |
			 +---------------+---------------+---------------+---------------+
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
		    NalByte |= (0  << 6) & 0b11000000;
		    NalByte |= (28 << 0) & 0b00111111;
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
				 |S|E|R|  Type   |
				 +---------------+

				S: 1 bit
					When set to one, the Start bit indicates the start of a
					fragmented NAL unit.  When the following FU payload is not the
					start of a fragmented NAL unit payload, the Start bit is set
					to zero.

				E: 1 bit
					When set to one, the End bit indicates the end of a fragmented
					NAL unit, i.e., the last byte of the payload is also the last
					byte of the fragmented NAL unit.  When the following FU
					payload is not the last fragment of a fragmented NAL unit, the
					End bit is set to zero.

				R: 1 bit
					The Reserved bit MUST be equal to 0 and MUST be ignored by the
					receiver.

				Type: 5 bits
					The NAL unit payload type as defined in Table 7-1 of [`ITU-T Recommendation H.264, "Advanced video coding for generic audiovisual services", March 2010.`]
				*/

				std::vector<uint8_t> FUHeader;
				// clang-format off
			    uint8_t FUHeaderByte = 0;
			    FUHeaderByte |= (bIsFirst    << 7) & 0b10000000;
			    FUHeaderByte |= (bIsLast     << 6) & 0b01000000;
                FUHeaderByte |= (0           << 5) & 0b00100000;
			    FUHeaderByte |= (NalUnitType << 0) & 0b00011111;
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

std::vector<RTPPacket> AvcPacketizer::Flush()
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

#undef LogAvcPacketizer