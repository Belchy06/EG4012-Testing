#include "ovb_common/common.h"

#include "ovb_send/packetizer/hevc/hevc_packetizer.h"

#define LogHevcPacketizer "LogHevcPacketizer"

HevcPacketizer::HevcPacketizer()
{
}

void HevcPacketizer::Packetize(std::vector<NALU> InNALUs)
{
	// https://datatracker.ietf.org/doc/html/rfc7798
	for (size_t i = 0; i < InNALUs.size(); i++)
	{
		NALU& Nal = InNALUs[i];

		assert(Nal.Size > 0);
		// clang-format off
	    uint8_t ForbiddenBit       = (Nal.Data[0] & 0b10000000) >> 7;
	    uint8_t NalUnitType        = (Nal.Data[0] & 0b01111110) >> 1;
		uint8_t LayerId            = (Nal.Data[0] & 0b00000001) << 5;
		        LayerId           |= (Nal.Data[1] & 0b11111000) >> 3;
		uint8_t NuhTemporalIdPlus1 = (Nal.Data[1] & 0b00000111) >> 0;
		// clang-format on
		assert(ForbiddenBit == 0);

		LOG(LogHevcPacketizer, LOG_SEVERITY_DETAILS, "Packetizing NAL. Type: {}({}); Size: {}", NalTypeToString(NalUnitType).c_str(), +NalUnitType, Nal.Size);

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
				APByte |= (0           << 7) & 0b10000000;
		    	APByte |= ((uint8_t)48 << 1) & 0b01111110;
				APByte |= (LayerId     >> 5) & 0b00000001;
		    	RTPPayload.push_back(APByte);
				// clang-format on

				// clang-format off
		    	APByte = 0;
				APByte |= (LayerId            << 3) & 0b11111000;
		    	APByte |= (NuhTemporalIdPlus1 << 0) & 0b00000111;
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
			NalByte |= (0       << 7) & 0b10000000;
		    NalByte |= (49      << 1) & 0b01111110;
			NalByte |= (LayerId >> 5) & 0b00000001;
		    NALHeader.push_back(NalByte);
			// clang-format on

			// clang-format off
		    NalByte = 0;
			NalByte |= (LayerId            << 3) & 0b11111000;
		    NalByte |= (NuhTemporalIdPlus1 << 0) & 0b00000111;
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
				 |S|E|  FuType   |
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

				FuType: 6 bits
					The field FuType MUST be equal to the field Type of the fragmented
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

std::vector<RTPPacket> HevcPacketizer::Flush()
{
	if (AggregatedPacket.GetPayloadSize() > 0)
	{
		Packets.push_back(RTPPacket(10, (size_t)SequenceNumber++, 0, AggregatedPacket.GetPayload(), AggregatedPacket.GetPayloadSize(), false));
		AggregatedPacket = RTPPacket();
	}

	std::vector<RTPPacket> ReturnPackets = Packets;
	Packets.clear();
	return ReturnPackets;
}

std::string HevcPacketizer::NalTypeToString(uint8_t InNalType)
{
	switch (InNalType)
	{
		case 0:
			return "TRAIL_N";
		case 1:
			return "TRAIL_R";
		case 2:
			return "TSA_N";
		case 3:
			return "TSA_R";
		case 4:
			return "STSA_N";
		case 5:
			return "STSA_R";
		case 6:
			return "RADL_N";
		case 7:
			return "RADL_R";
		case 8:
			return "RASL_N";
		case 9:
			return "RASL_R";
		case 10:
			return "RSV_VCL_N10";
		case 11:
			return "RSV_VCL_N12";
		case 12:
			return "RSV_VCL_N14";
		case 13:
			return "RSV_VCL_R11";
		case 14:
			return "RSV_VCL_R13";
		case 15:
			return "RSV_VCL_R15";
		case 16:
			return "BLA_W_LP";
		case 17:
			return "BLA_W_RADL";
		case 18:
			return "BLA_N_LP";
		case 19:
			return "IDR_W_RADL";
		case 20:
			return "IDR_N_LP";
		case 21:
			return "CRA_NUT";
		case 22:
			return "RSV_IRAP_VCL22";
		case 23:
			return "RSV_IRAP_VCL23";
		case 24:
			return "RSV_VCL24";
		case 25:
			return "RSV_VCL25";
		case 26:
			return "RSV_VCL26";
		case 27:
			return "RSV_VCL27";
		case 28:
			return "RSV_VCL28";
		case 29:
			return "RSV_VCL29";
		case 30:
			return "RSV_VCL30";
		case 31:
			return "RSV_VCL31";
		case 32:
			return "VPS_NUT";
		case 33:
			return "SPS_NUT";
		case 34:
			return "PPS_NUT";
		case 35:
			return "AUD_NUT";
		case 36:
			return "EOS_NUT";
		case 37:
			return "EOB_NUT";
		case 38:
			return "FD_NUT";
		case 39:
			return "PREFIX_SEI_NUT";
		case 40:
			return "SUFFIX_SEI_NUT";
			// 41 .. 47 RSV_NVCL
			// 48 .. 63 Unspecified
		default:
			return "UNKNOWN";
	}
}

#undef LogHevcPacketizer