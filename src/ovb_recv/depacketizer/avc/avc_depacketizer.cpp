#include "ovb_common/common.h"
#include "ovb_recv/depacketizer/avc/avc_depacketizer.h"

#define LogAvcDepacketizer "LogAvcDepacketizer"

void AvcDepacketizer::HandlePacket(RTPPacket InPacket)
{
	if (InPacket.GetSequenceNumber() < PrevSequenceNumber)
	{
		// A packet has come in late, discard it
		return;
	}

	if (InPacket.GetSequenceNumber() > PrevSequenceNumber + 1)
	{
		// A packet has come in with a sequence number higher than expected, notify users of the number of packets missed
		LOG(LogAvcDepacketizer, LOG_SEVERITY_WARNING, "Missed packet(s): {} -> {}", PrevSequenceNumber + 1, InPacket.GetSequenceNumber() - 1);
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
	assert(PacketSize > 2);

	// clang-format off
	// clang-format off
	uint8_t StartByte   = (PacketData[0] & 0b11111111) >> 0;
	uint8_t FBit        = (StartByte     & 0b10000000) >> 7;
	// uint8_t NRI         = (StartByte     & 0b01100000) >> 5;
	uint8_t NalUnitType = (StartByte     & 0b00011111) >> 0;
	// clang-format on
	assert(FBit == 0);

	LOG(LogAvcDepacketizer, LOG_SEVERITY_DETAILS, "Depacketizing NAL. Type: {}; Size: {}", +NalUnitType, PacketSize);

	if (NalUnitType != 24 && NalUnitType != 28)
	{
		if (Fragments.size() > 0)
		{
			// We've received a new single nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogAvcDepacketizer, LOG_SEVERITY_WARNING, "Received new single nal without finishing previously fragmented nal. Dropping {} fragments", Fragments.size());
			Fragments.clear();
		}

		// Single nal
		DepacketizerListener->OnNALReceived(PacketData, PacketSize);
	}
	else if (NalUnitType == 24)
	{
		if (Fragments.size() > 0)
		{
			// We've received a new AP without finishing off the last fragmented unit. Warn and continue
			LOG(LogAvcDepacketizer, LOG_SEVERITY_WARNING, "Received new fragmented nal without finishing previously fragmented nal. Dropping {} fragments", Fragments.size());
			Fragments.clear();
		}

		// Skip PayloadHdr
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
	else if (NalUnitType == 28)
	{
		// Skip PayloadHdr
		PacketData += 1;
		PacketSize -= 1;

		// Fragmentation packet
		uint8_t FUHeaderByte = PacketData[0];
		// clang-format off
		uint8_t bIsFirst = (FUHeaderByte & 0b10000000) >> 7;
		uint8_t bIsLast  = (FUHeaderByte & 0b01000000) >> 6;
        uint8_t Reserved  =(FUHeaderByte & 0b00100000) >> 5;
		// uint8_t FUType =   (FUHeaderByte & 0b00011111) >> 0;
		// clang-format on
		assert(Reserved == 0);

		if (bIsFirst && Fragments.size() > 0)
		{
			// We've received a new fragmented nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogAvcDepacketizer, LOG_SEVERITY_WARNING, "Received new fragmented nal without finishing previously fragmented nal. Dropping {} fragments", Fragments.size());
			Fragments.clear();
		}

		Fragments.push_back(InPacket);

		if (bIsLast)
		{
			std::vector<uint8_t> ReconstructedNalBytes;

			for (RTPPacket Fragment : Fragments)
			{
				uint8_t* FragmentData = Fragment.GetPayload();
				// Skip FU Header
				for (size_t i = 1; i < Fragment.GetPayloadSize(); i++)
				{
					ReconstructedNalBytes.push_back(FragmentData[i]);
				}
			}

			DepacketizerListener->OnNALReceived(ReconstructedNalBytes.data(), ReconstructedNalBytes.size());
			Fragments.clear();
		}
	}
}

#undef LogAvcDepacketizer