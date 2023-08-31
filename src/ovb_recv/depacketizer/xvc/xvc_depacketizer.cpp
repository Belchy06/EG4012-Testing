#include "ovb_recv/depacketizer/xvc/xvc_depacketizer.h"

#include "ovb_common/common.h"

#define LogXvcDepacketizer "LogXvcDepacketizer"

void XvcDepacketizer::HandlePacket(RTPPacket InPacket)
{
	// TODO (belchy06): Reordering?

	if (InPacket.GetSequenceNumber() < PrevSequenceNumber)
	{
		// A packet has come in late, discard it
		return;
	}

	if (InPacket.GetSequenceNumber() > PrevSequenceNumber + 1)
	{
		// A packet has come in with a sequence number higher than expected, notify users of the number of packets missed
		LOG(LogXvcDepacketizer, LOG_SEVERITY_WARNING, "Missed packet(s): {} -> {}", PrevSequenceNumber + 1, InPacket.GetSequenceNumber() - 1);
	}

	// if (InPacket.GetTimeStamp() > prevTimestamp)
	// {
	// 	// A packet has come in with a newer timestamp (indicating a new NAL), but we haven't finished receiving the previous NAL
	// 	Packets.clear();
	// }

	PrevTimestamp = InPacket.GetTimeStamp();
	PrevSequenceNumber = InPacket.GetSequenceNumber();

	uint8_t* PacketData = InPacket.GetPayload();
	size_t	 PacketSize = InPacket.GetPayloadSize();
	assert(PacketSize > 1);

	// clang-format off
	uint8_t NalUnitType =        (PacketData[0] & 0b00111111) >> 0;
	// clang-format on
	LOG(LogXvcDepacketizer, LOG_SEVERITY_DETAILS, "Depacketizing NAL. Type: {}; Size: {}", +NalUnitType, PacketSize);

	if (NalUnitType != 48 && NalUnitType != 49)
	{
		// Single nal
		if (Fragments.size() > 0)
		{
			// We've received a new single nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogXvcDepacketizer, LOG_SEVERITY_WARNING, "Received new single nal without finishing previously fragmented nal. Dropping {} fragments", Fragments.size());
			Fragments.clear();
		}

		DepacketizerListener->OnNALReceived(PacketData + 1, PacketSize - 1);
	}
	else if (NalUnitType == 48)
	{
		// Aggregation Packet
		if (Fragments.size() > 0)
		{
			// We've received a new single nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogXvcDepacketizer, LOG_SEVERITY_WARNING, "Received new aggregation packet without finishing previously fragmented nal. Dropping {} fragments", Fragments.size());
			Fragments.clear();
		}

		PacketData += 1;
		PacketSize -= 1;

		while (PacketSize > 0)
		{
			size_t	 ReadIdx = 0;
			uint16_t NALSize = 0;
			NALSize |= PacketData[ReadIdx++] << 8;
			NALSize |= PacketData[ReadIdx++] << 0;
			// Skip NALSize
			PacketData += 2;
			PacketSize -= 2;

			DepacketizerListener->OnNALReceived(PacketData, NALSize);

			PacketData += NALSize;
			PacketSize -= NALSize;
		}
	}
	else if (NalUnitType == 49)
	{
		// Fragmentaion packet
		uint8_t FUHeaderByte = PacketData[1];
		// clang-format off
		uint8_t bIsFirst = (FUHeaderByte & 0b10000000) >> 6;
		uint8_t bIsLast  = (FUHeaderByte & 0b01000000) >> 5;
		// clang-format on

		if (bIsFirst && Fragments.size() > 0)
		{
			// We've received a new fragmented nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogXvcDepacketizer, LOG_SEVERITY_WARNING, "Received new fragmented nal without finishing previously fragmented nal. Dropping {} fragments", Fragments.size());
			Fragments.clear();
		}

		Fragments.push_back(InPacket);

		if (bIsLast)
		{
			std::vector<uint8_t> ReconstructedNalBytes;

			for (RTPPacket Fragment : Fragments)
			{
				uint8_t* FragmentData = Fragment.GetPayload();
				for (size_t i = 2; i < Fragment.GetPayloadSize(); i++)
				{
					ReconstructedNalBytes.push_back(FragmentData[i]);
				}
			}

			DepacketizerListener->OnNALReceived(ReconstructedNalBytes.data(), ReconstructedNalBytes.size());

			Fragments.clear();
		}
	}
}

#undef LogXvcDepacketizer