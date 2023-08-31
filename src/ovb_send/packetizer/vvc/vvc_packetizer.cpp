#include <assert.h>
#include <iostream>

#include "ovb_common/common.h"
#include "ovb_send/packetizer/vvc/vvc_packetizer.h"

#define LogVvcPacketizer "LogVvcPacketizer"

VvcPacketizer::VvcPacketizer()
{
}

void VvcPacketizer::Packetize(std::vector<NALU> InNALUs)
{
	for (size_t i = 0; i < InNALUs.size(); i++)
	{
		NALU& Nal = InNALUs[i];
		assert(Nal.Size > 0);

		if (Nal.Data[2] == 1)
		{
			Nal.Data += 3;
			Nal.Size -= 3;
		}
		else if (Nal.Data[3] == 1)
		{
			Nal.Data += 4;
			Nal.Size -= 4;
		}
		else
		{
			// TODO (belchy06): Handle gracefully
			unimplemented();
		}

		// clang-format off
	    uint8_t ForbiddenZeroBit =   (Nal.Data[0] & 0b10000000) >> 7;
	    uint8_t NuhReservedZeroBit = (Nal.Data[0] & 0b01000000) >> 6;
	    uint8_t NuhLayerId =         (Nal.Data[0] & 0b00111111) >> 0;
	    uint8_t NalUnitType =        (Nal.Data[1] & 0b11111000) >> 3;
	    uint8_t NuhTemporalIdPlus1 = (Nal.Data[1] & 0b00000111) >> 0;
		// clang-format on
		assert(ForbiddenZeroBit == 0);
		assert(NuhReservedZeroBit == 0);
		LOG(LogVvcPacketizer, LOG_SEVERITY_DETAILS, "Packetizing NAL. Type: {}; Size: {}", +NalUnitType, Nal.Size);

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
				// Add NAL to new AP
				std::vector<uint8_t> RTPPayload;

				// This is the first packet in the AP.
				uint8_t APByte;
				// clang-format off
                APByte = 0;
		        APByte |= (ForbiddenZeroBit   << 7) & 0b10000000;
		        APByte |= (NuhReservedZeroBit << 6) & 0b01000000;
		        APByte |= (NuhLayerId         << 0) & 0b00111111;
                RTPPayload.push_back(APByte);
				// clang-format on

				// clang-format off
                APByte = 0;
                //       AP Type
		        APByte |= (28                << 3) & 0b11111000;
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
				// Append NAL to existing AP
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

				// Single NAL (4.3.1 https://www.rfc-editor.org/rfc/rfc9328.pdf)
				RTPPacket Packet = RTPPacket(10, (size_t)SequenceNumber++, 0, Nal.Data, Nal.Size, false);
				// TODO (belchy06): RTP Timestamp
				Packets.push_back(Packet);
			}
		}
		else
		{
			// Fragmentation unit
			if (AggregatedPacket.GetPayloadSize() > 0)
			{
				Packets.push_back(RTPPacket(10, (size_t)SequenceNumber++, 0, AggregatedPacket.GetPayload(), AggregatedPacket.GetPayloadSize(), false));
				AggregatedPacket = RTPPacket();
			}

			// Update size to remove the two byte NAL header as we'll be re-writing this ourself
			Nal.Data += 2;
			Nal.Size -= 2;
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
			while (SizePacketized < Nal.Size)
			{
				size_t	PacketSize = std::min(MaxSize, Nal.Size - SizePacketized);
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

		if (AggregatedPacket.GetPayloadSize() + Nal.Size < RTP_PAYLOAD_SIZE)
		{
		}
		else if (Nal.Size < RTP_PAYLOAD_SIZE)
		{
		}
		else
		{
		}
	}
}

std::vector<RTPPacket> VvcPacketizer::Flush()
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

#undef LogVvcPacketizer