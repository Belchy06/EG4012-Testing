#pragma once

#include <stdint.h>

class RTPPacket
{
public:
	RTPPacket(int InPType, int InFrameNB, int InTime, const uint8_t* InData, int InDataLength);
	RTPPacket(uint8_t* InData, int InDataLength);

private:
private:
	// Size of the RTP header:
	static const int HEADER_SIZE = 12;

	// Fields that compose the RTP header
	int Version;
	int Padding;
	int Extension;
	int CC;
	int Marker;
	int PayloadType;
	int SequenceNumber;
	int TimeStamp;
	int Ssrc;

	// Bitstream of the RTP header
	uint8_t* Header;

	// Size of the RTP payload
	int PayloadSize;

	// Bitstream of the RTP payload
	uint8_t* Payload;
};