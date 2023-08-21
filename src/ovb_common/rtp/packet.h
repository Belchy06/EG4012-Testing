#pragma once

#include <stdint.h>

class RTPPacket
{
public:
	RTPPacket(uint8_t InPType, uint16_t InFrameNB, uint32_t InTime, const uint8_t* InData, size_t InDataLength, bool InbIsLast);
	RTPPacket(const uint8_t* InData, int64_t InDataLength);

	uint8_t* GetHeader();
	size_t	 GetHeaderSize();
	uint8_t* GetPayload();
	size_t	 GetPayloadSize();

	// Fields that compose the RTP header
	int		 GetVersion();
	int		 GetPadding();
	int		 GetExtension();
	int		 GetCC();
	int		 GetMarker();
	uint8_t	 GetPayloadType();
	uint16_t GetSequenceNumber();
	uint32_t GetTimeStamp();
	int		 GetSsrc();

private:
	// Fields that compose the RTP header
	int		 Version;
	int		 Padding;
	int		 Extension;
	int		 CC;
	int		 Marker;
	uint8_t	 PayloadType;
	uint16_t SequenceNumber;
	uint32_t TimeStamp;
	int		 Ssrc;

	// Bitstream of the RTP header
	uint8_t* Header;
	// Size of the RTP header:
	static const size_t HEADER_SIZE = 12;
	// Size of the RTP payload
	size_t PayloadSize;
	// Bitstream of the RTP payload
	uint8_t* Payload;
};