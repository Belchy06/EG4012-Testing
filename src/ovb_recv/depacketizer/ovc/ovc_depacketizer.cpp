#include "ovb_common/common.h"
#include "ovb_recv/depacketizer/ovc/ovc_depacketizer.h"

#define LogOvcDepacketizer "LogOvcDepacketizer"

void OvcDepacketizer::HandlePacket(RTPPacket InPacket)
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
		LOG(LogOvcDepacketizer, LOG_SEVERITY_WARNING, "Missed packet(s): %d -> %d", PrevSequenceNumber + 1, InPacket.GetSequenceNumber() - 1);
	}

	// if (InPacket.GetTimeStamp() > prevTimestamp)
	// {
	// 	// A packet has come in with a newer timestamp (indicating a new NAL), but we haven't finished receiving the previous NAL
	// 	Packets.clear();
	// }

	uint8_t* PacketData = InPacket.GetPayload();
	size_t	 PacketSize = InPacket.GetPayloadSize();
	assert(PacketSize > 2);

	// clang-format off
	uint8_t StartByte   = (PacketData[0] & 0b11111111) >> 0;
	uint8_t ZeroBits    = (PacketData[1] & 0b11000000) >> 6;
	uint8_t NalUnitType = (PacketData[1] & 0b00111111) >> 0;
	// clang-format on
	// assert(StartByte == 0);
	// assert(ZeroBits == 0);

	LOG(LogOvcDepacketizer, LOG_SEVERITY_DETAILS, "Depacketizing NAL. Type: %d; Size: %d", +NalUnitType, PacketSize);

	if (NalUnitType != 49)
	{
		// Assume single nal, no aggregration packets
		if (Fragments.size() > 0)
		{
			// We've received a new single nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogOvcDepacketizer, LOG_SEVERITY_WARNING, "Received new single nal without finishing previously fragmented nal. Dropping %d fragments", Fragments.size());
			Fragments.clear();
		}

		DepacketizerListener->OnNALReceived(PacketData, PacketSize);
	}
	else if (NalUnitType == 49)
	{
		// Fragmentaion packet
		uint8_t FUHeaderByte = PacketData[2];
		// clang-format off
		uint8_t bIsFirst = (FUHeaderByte & 0b10000000) >> 7;
		uint8_t bIsLast  = (FUHeaderByte & 0b01000000) >> 6;
		uint8_t FUType =   (FUHeaderByte & 0b00111111) >> 0;
		// clang-format on

		if (bIsFirst && Fragments.size() > 0)
		{
			// We've received a new fragmented nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogOvcDepacketizer, LOG_SEVERITY_WARNING, "Received new fragmented nal without finishing previously fragmented nal. Dropping %d fragments", Fragments.size());
			Fragments.clear();
		}

		Fragments.push_back(InPacket);

		if (bIsLast)
		{
			std::vector<uint8_t> ReconstructedNalBytes;

			std::vector<uint8_t> NalHeader;
			uint8_t				 NalHeaderByte;
			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= StartByte;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= (ZeroBits << 6) & 0b11000000;
            NalHeaderByte |= (FUType   << 0) & 0b00111111;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			ReconstructedNalBytes.reserve(NalHeader.size() + ReconstructedNalBytes.size());
			ReconstructedNalBytes.insert(ReconstructedNalBytes.end(), NalHeader.begin(), NalHeader.end());
			for (RTPPacket Fragment : Fragments)
			{
				uint8_t* FragmentData = Fragment.GetPayload();
				for (size_t i = 3; i < Fragment.GetPayloadSize(); i++)
				{
					ReconstructedNalBytes.push_back(FragmentData[i]);
				}
			}

			DepacketizerListener->OnNALReceived(ReconstructedNalBytes.data(), ReconstructedNalBytes.size());

			PrevTimestamp = Fragments[0].GetTimeStamp();
			PrevSequenceNumber = Fragments[0].GetSequenceNumber();

			Fragments.clear();
		}
	}
}

#undef LogOvcDepacketizer