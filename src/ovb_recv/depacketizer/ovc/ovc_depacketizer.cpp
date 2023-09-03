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
		LOG(LogOvcDepacketizer, LOG_SEVERITY_WARNING, "Missed packet(s): {} -> {}", PrevSequenceNumber + 1, InPacket.GetSequenceNumber() - 1);
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
	uint8_t StartByte   = (PacketData[0] & 0b11111111) >> 0;
	uint8_t ZeroBits    = (PacketData[1] & 0b11000000) >> 6;
	uint8_t NalUnitType = (PacketData[1] & 0b00111111) >> 0;
	// clang-format on
	// assert(StartByte == 0);
	// assert(ZeroBits == 0);

	LOG(LogOvcDepacketizer, LOG_SEVERITY_DETAILS, "Depacketizing NAL. Type: {}; Size: {}", +NalUnitType, PacketSize);

	if (NalUnitType != 49 && NalUnitType != 48)
	{
		// Single nal
		if (Fragments.size() > 0)
		{
			// We've received a new single nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogOvcDepacketizer, LOG_SEVERITY_WARNING, "Received new fragmented nal without finishing previously fragmented nal. Reconstructing previous {} fragments", Fragments.size());

			uint8_t* FragmentData = Fragments[0].GetPayload();

			// clang-format off
			uint8_t OldStartByte   = (FragmentData[0] & 0b11111111) >> 0;
			uint8_t OldZeroBits    = (FragmentData[1] & 0b11000000) >> 6;
			// clang-format off

			// Fragmentaion packet
			uint8_t OldFUHeaderByte = FragmentData[2];
			// clang-format off
			uint8_t OldFUType = (OldFUHeaderByte & 0b00111111) >> 0;
			// clang-format on

			std::vector<uint8_t> ReconstructedNalBytes;

			std::vector<uint8_t> NalHeader;
			uint8_t				 NalHeaderByte;
			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= OldStartByte;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= (OldZeroBits << 6) & 0b11000000;
            NalHeaderByte |= (OldFUType   << 0) & 0b00111111;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			ReconstructedNalBytes.reserve(NalHeader.size() + ReconstructedNalBytes.size());
			ReconstructedNalBytes.insert(ReconstructedNalBytes.end(), NalHeader.begin(), NalHeader.end());
			for (RTPPacket Fragment : Fragments)
			{
				for (size_t i = 3; i < Fragment.GetPayloadSize(); i++)
				{
					ReconstructedNalBytes.push_back(Fragment.GetPayload()[i]);
				}
			}

			DepacketizerListener->OnNALReceived(ReconstructedNalBytes.data(), ReconstructedNalBytes.size());
			Fragments.clear();
		}

		DepacketizerListener->OnNALReceived(PacketData, PacketSize);
	}
	else if (NalUnitType == 48)
	{
		if (Fragments.size() > 0)
		{
			// We've received a new AP without finishing off the last fragmented unit. Warn and continue
			LOG(LogOvcDepacketizer, LOG_SEVERITY_WARNING, "Received new aggregated packet without finishing previously fragmented nal. Reconstructing previous {} fragments", Fragments.size());

			uint8_t* FragmentData = Fragments[0].GetPayload();

			// clang-format off
			uint8_t OldStartByte   = (FragmentData[0] & 0b11111111) >> 0;
			uint8_t OldZeroBits    = (FragmentData[1] & 0b11000000) >> 6;
			// clang-format off

			// Fragmentaion packet
			uint8_t OldFUHeaderByte = FragmentData[2];
			// clang-format off
			uint8_t OldFUType = (OldFUHeaderByte & 0b00111111) >> 0;
			// clang-format on

			std::vector<uint8_t> ReconstructedNalBytes;

			std::vector<uint8_t> NalHeader;
			uint8_t				 NalHeaderByte;
			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= OldStartByte;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= (OldZeroBits << 6) & 0b11000000;
            NalHeaderByte |= (OldFUType   << 0) & 0b00111111;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			ReconstructedNalBytes.reserve(NalHeader.size() + ReconstructedNalBytes.size());
			ReconstructedNalBytes.insert(ReconstructedNalBytes.end(), NalHeader.begin(), NalHeader.end());
			for (RTPPacket Fragment : Fragments)
			{
				for (size_t i = 3; i < Fragment.GetPayloadSize(); i++)
				{
					ReconstructedNalBytes.push_back(Fragment.GetPayload()[i]);
				}
			}

			DepacketizerListener->OnNALReceived(ReconstructedNalBytes.data(), ReconstructedNalBytes.size());
			Fragments.clear();
		}

		// Skip PayloadHdr
		PacketData += 2;
		PacketSize -= 2;

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
		uint8_t FUHeaderByte = PacketData[2];
		// clang-format off
		uint8_t bIsFirst = (FUHeaderByte & 0b10000000) >> 7;
		uint8_t bIsLast  = (FUHeaderByte & 0b01000000) >> 6;
		uint8_t FUType =   (FUHeaderByte & 0b00111111) >> 0;
		// clang-format on

		if (bIsFirst && Fragments.size() > 0)
		{
			// We've received a new fragmented nal without finishing off the last fragmented unit. Warn and continue
			LOG(LogOvcDepacketizer, LOG_SEVERITY_WARNING, "Received new fragmented nal without finishing previously fragmented nal. Reconstructing previous {} fragments", Fragments.size());

			uint8_t* FragmentData = Fragments[0].GetPayload();

			// clang-format off
			uint8_t OldStartByte   = (FragmentData[0] & 0b11111111) >> 0;
			uint8_t OldZeroBits    = (FragmentData[1] & 0b11000000) >> 6;
			// clang-format off

			// Fragmentaion packet
			uint8_t OldFUHeaderByte = FragmentData[2];
			// clang-format off
			uint8_t OldFUType = (OldFUHeaderByte & 0b00111111) >> 0;
			// clang-format on

			std::vector<uint8_t> ReconstructedNalBytes;

			std::vector<uint8_t> NalHeader;
			uint8_t				 NalHeaderByte;
			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= OldStartByte;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			// clang-format off
			NalHeaderByte = 0;
            NalHeaderByte |= (OldZeroBits << 6) & 0b11000000;
            NalHeaderByte |= (OldFUType   << 0) & 0b00111111;
            NalHeader.push_back(NalHeaderByte);
			// clang-format on

			ReconstructedNalBytes.reserve(NalHeader.size() + ReconstructedNalBytes.size());
			ReconstructedNalBytes.insert(ReconstructedNalBytes.end(), NalHeader.begin(), NalHeader.end());
			for (RTPPacket Fragment : Fragments)
			{
				for (size_t i = 3; i < Fragment.GetPayloadSize(); i++)
				{
					ReconstructedNalBytes.push_back(Fragment.GetPayload()[i]);
				}
			}

			DepacketizerListener->OnNALReceived(ReconstructedNalBytes.data(), ReconstructedNalBytes.size());
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
			Fragments.clear();
		}
	}
}

#undef LogOvcDepacketizer