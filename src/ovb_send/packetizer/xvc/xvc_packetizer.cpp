#include "ovb_common/common.h"
#include "ovb_send/packetizer/xvc/xvc_packetizer.h"

#include "xvc_common_lib/common.h"
#include "xvc_common_lib/picture_types.h"

#define LogXvcPacketizer "LogXvcPacketizer"

XvcPacketizer::XvcPacketizer()
{
}

void XvcPacketizer::Packetize(std::vector<NALU> InNALUs)
{
	// No RTP Spec is defined for XVC. For this case, we're going to use
	// something similiar to HEVC's:
	// https://datatracker.ietf.org/doc/html/rfc7798#section-4.4.3
	for (size_t i = 0; i < InNALUs.size(); i++)
	{
		NALU& Nal = InNALUs[i];

		assert(Nal.Size > 0);

		uint8_t Header = Nal.Data[0];
		uint8_t XvcBitOne = ((Header >> 7) & 0b1);
		if (XvcBitOne == 0)
		{
			// clang-format off
		    if ((xvc::NalUnitType)((Header >> 1) & 0b11111) == xvc::NalUnitType::kIntraAccessPicture || 
                (xvc::NalUnitType)((Header >> 1) & 0b11111) == xvc::NalUnitType::kPredictedPicture || 
                (xvc::NalUnitType)((Header >> 1) & 0b11111) == xvc::NalUnitType::kBipredictedPicture || 
                (xvc::NalUnitType)((Header >> 1) & 0b11111) == xvc::NalUnitType::kSegmentHeader)
			// clang-format on
			{
				// The above NAL unit types of xvc version 1 bitstreams are allowed
				// to not have xvc_bit_one set to 1.
			}
			else if (Header == xvc::constants::kEncapsulationCode)
			{
				// Nal units with an encapsulation code consisting of two bytes
				Header = Nal.Data[2];
			}
			else
			{
				// Early out
				continue;
			}
		}

		uint8_t NalRFE = ((Header >> 6) & 0b00000001);
		if (NalRFE == 1)
		{
			// Early out
			continue;
		}

		uint8_t NalUnitType = ((Header >> 1) & 0b00011111);
		LOG(LogXvcPacketizer, LOG_SEVERITY_DETAILS, "Packetizing NAL. Type: {}; Size: {}", +NalUnitType, Nal.Size);

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
				 |Header(Type=48)|          NAL 1 Size           | NAL 1 Data ...|
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
				 |Header(Type=48)|          NAL 1 Size           | NAL 1 Data ...|
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
				std::vector<uint8_t> NalBytes;
				uint8_t				 PayloadHeaderByte;
				PayloadHeaderByte = 0;
				PayloadHeaderByte |= ((NalUnitType >> 0) & 0b00011111);
				NalBytes.push_back(NalUnitType);

				for (size_t j = 0; j < Nal.Size; j++)
				{
					NalBytes.push_back(Nal.Data[j]);
				}

				RTPPacket Packet = RTPPacket(10, (size_t)SequenceNumber++, 0, NalBytes.data(), NalBytes.size(), false);
				// TODO (belchy06): RTP Timestamp
				Packets.push_back(Packet);
			}
		}
		else
		{
			/*
			 +---------------+---------------+---------------+---------------+
			 |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
			 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 |Header(Type=49)|   FU Header   |          FU Payload ...       |
			 +---------------+---------------+                               +
			 |                              ...                              |
			 +---------------+---------------+---------------+---------------+

			*/

			// Fragmentation Unit
			std::vector<uint8_t> NALHeader;
			uint8_t				 NalByte;
			// clang-format off
            NalByte = 0;
            //       FU Type
		    NalByte |= (49        << 0) & 0b00111111;
            NALHeader.push_back(NalByte);
			// clang-format on

			size_t	SizePacketized = 0;
			uint8_t bIsFirst = 1;
			size_t	MaxSize = (size_t)RTP_PAYLOAD_SIZE - 2;
			while (SizePacketized < Nal.Size)
			{
				size_t	PacketSize = std::min(MaxSize, Nal.Size - SizePacketized);
				uint8_t bIsLast = PacketSize < MaxSize;

				/*
				The Structure of an FU Header

				 +---------------+
				 |0|1|2|3|4|5|6|7|
				 +-+-+-+-+-+-+-+-+
				 |S|E|  FU Type  |
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

				FU Type: 6 bits
					The field FU Type MUST be equal to the field Type of the fragmented
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

std::vector<RTPPacket> XvcPacketizer::Flush()
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

#undef LogXvcPacketizer