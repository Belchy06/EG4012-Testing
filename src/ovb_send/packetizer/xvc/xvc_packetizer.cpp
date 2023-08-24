#include "ovb_common/common.h"
#include "ovb_send/packetizer/xvc/xvc_packetizer.h"

#include "xvc_common_lib/common.h"
#include "xvc_common_lib/picture_types.h"

#define LogXvcPacketizer "LogXvcPacketizer"

std::vector<RTPPacket> XvcPacketizer::Packetize(uint8_t* InData, size_t InSize)
{
	// No RTP Spec is defined for XVC. For this case, we're going to use
	// something similiar to HEVC's:
	// https://datatracker.ietf.org/doc/html/rfc7798#section-4.4.3
	assert(InSize > 0);

	std::vector<RTPPacket> Packets;

	uint8_t Header = InData[0];
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
			Header = InData[2];
		}
		else
		{
			// Early out
			return Packets;
		}
	}

	uint8_t NalRFE = ((Header >> 6) & 0b00000001);
	if (NalRFE == 1)
	{
		// Early out
		return Packets;
	}

	uint8_t NalUnitType = ((Header >> 1) & 0b00011111);
	LOG(LogXvcPacketizer, LOG_SEVERITY_DETAILS, "Packetizing NAL. Type: %d; Size: %d", +NalUnitType, InSize);

	if (InSize < RTP_PAYLOAD_SIZE)
	{
		// Single NAL
		// TODO (belchy06): Aggregation Packets
		std::vector<uint8_t> NalBytes;
		uint8_t				 PayloadHeaderByte;
		PayloadHeaderByte = 0;
		PayloadHeaderByte |= ((NalUnitType >> 0) & 0b00011111);
		NalBytes.push_back(NalUnitType);

		for (size_t i = 0; i < InSize; i++)
		{
			NalBytes.push_back(InData[i]);
		}

		RTPPacket Packet = RTPPacket(10, (size_t)SequenceNumber++, 0, NalBytes.data(), NalBytes.size(), false);
		// TODO (belchy06): RTP Timestamp
		Packets.push_back(Packet);
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
        //             FU Type
		NalByte |= ((uint8_t)49        << 0) & 0b11111111;
        NALHeader.push_back(NalByte);
		// clang-format on

		size_t	SizePacketized = 0;
		uint8_t bIsFirst = 1;
		size_t	MaxSize = (size_t)RTP_PAYLOAD_SIZE - 2;
		while (SizePacketized < InSize)
		{
			size_t	PacketSize = std::min(MaxSize, InSize - SizePacketized);
			uint8_t bIsLast = PacketSize < MaxSize;

			/*
			The Structure of an FU Header

			 +---------------+
			 |0|1|2|3|4|5|6|7|
			 +-+-+-+-+-+-+-+-+
			 |Z|S|E| FU Type |
			 +---------------+

			Z: 1 bit
				Forbidden_zero_bit. This feld is required to be zero in XVC.

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

			FU Type: 5 bits
				The field FU Type MUST be equal to the field Type of the fragmented
				NAL unit.
			*/
			std::vector<uint8_t> FUHeader;
			// clang-format off
			uint8_t FUHeaderByte = 0;
            FUHeaderByte |= (0           << 7) & 0b10000000;
            FUHeaderByte |= (bIsFirst    << 6) & 0b01000000;
            FUHeaderByte |= (bIsLast     << 5) & 0b00100000;
            FUHeaderByte |= (NalUnitType << 0) & 0b00011111;
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

#undef LogXvcPacketizer