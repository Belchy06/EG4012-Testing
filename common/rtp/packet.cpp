#include <iostream>

#include "packet.h"

RTPPacket::RTPPacket(uint8_t InPType, uint16_t InFrameNB, uint32_t InTime, const uint8_t* InData, size_t InDataLength, bool InbIsLast)
{
	// Constant header fields:
	Version = 2;
	Padding = 0;
	Extension = 0;
	CC = 0;
	Ssrc = 0;
	// Variable header fields:
	SequenceNumber = InFrameNB;
	TimeStamp = InTime;
	PayloadType = InPType;
	Marker = InbIsLast ? 1 : 0;

	// Defined in https://www.cl.cam.ac.uk/~jac22/books/mm/book/node159.html
	// Construct header
	Header = new uint8_t[HEADER_SIZE];
	Header[0] = (uint8_t)((Version << 6) + (Padding << 5) + (Extension << 4) + CC);
	Header[1] = (uint8_t)((Marker << 7) + (PayloadType & ~0x80));
	Header[2] = (uint8_t)((SequenceNumber & 0xFF00) >> 8);
	Header[3] = (uint8_t)(SequenceNumber & 0xFF);
	Header[4] = (uint8_t)((TimeStamp >> 24) & 0xFF);
	Header[5] = (uint8_t)((TimeStamp >> 16) & 0xFF);
	Header[6] = (uint8_t)((TimeStamp >> 8) & 0xFF);
	Header[7] = (uint8_t)((TimeStamp >> 0) & 0xFF);
	Header[8] = (uint8_t)((Ssrc >> 24) & 0xFF);
	Header[9] = (uint8_t)((Ssrc >> 16) & 0xFF);
	Header[10] = (uint8_t)((Ssrc >> 8) & 0xFF);
	Header[11] = (uint8_t)((Ssrc >> 0) & 0xFF);

	// Construct payload
	PayloadSize = InDataLength;
	Payload = new uint8_t[InDataLength];

	for (int i = 0; i < InDataLength; i++)
	{
		Payload[i] = InData[i];
	}
}

RTPPacket::RTPPacket(const uint8_t* InData, int InDataLength)
{
	if (InDataLength < 0 || InDataLength < HEADER_SIZE)
	{
		std::cout << __FILE__ << ": Invalid data length (" << InDataLength << ")" << std::endl;
		return;
	}

	// get the header bitsream:
	Header = new uint8_t[HEADER_SIZE];
	for (int i = 0; i < HEADER_SIZE; i++)
	{
		Header[i] = InData[i];
	}
	Ssrc = 0;

	// get the payload bitstream:
	PayloadSize = InDataLength - HEADER_SIZE;
	Payload = new uint8_t[PayloadSize];
	for (int i = HEADER_SIZE; i < InDataLength; i++)
	{

		Payload[i - HEADER_SIZE] = InData[i];
	}

	// interpret the changing fields of the header:
	Version = (Header[0] >> 6) & 0b11;
	Padding = (Header[0] >> 5) & 0b1;
	Extension = (Header[0] >> 4) & 0b1;
	CC = Header[0] & 0b1111;
	Marker = (Header[1] >> 7) & 0b1;
	PayloadType = Header[1] & 0b1111111;
	SequenceNumber = (uint32_t)Header[3] + 256 * (uint32_t)Header[2];
	TimeStamp = (uint32_t)Header[7] + 256 * (uint32_t)Header[6] + 65536 * (uint32_t)Header[5] + 16777216 * (uint32_t)Header[4];
	Ssrc = (uint32_t)Header[8] + 256 * (uint32_t)Header[9] + 65536 * (uint32_t)Header[10] + 16777216 * (uint32_t)Header[11];
}

uint8_t* RTPPacket::GetHeader()
{
	return Header;
}

size_t RTPPacket::GetHeaderSize()
{
	return HEADER_SIZE;
}

uint8_t* RTPPacket::GetPayload()
{
	return Payload;
}

size_t RTPPacket::GetPayloadSize()
{
	return PayloadSize;
}

int RTPPacket::GetVersion()
{
	return Version;
}

int RTPPacket::GetPadding()
{
	return Padding;
}

int RTPPacket::GetExtension()
{
	return Extension;
}

int RTPPacket::GetCC()
{
	return CC;
}

int RTPPacket::GetMarker()
{
	return Marker;
}

uint8_t RTPPacket::GetPayloadType()
{
	return PayloadType;
}

uint16_t RTPPacket::GetSequenceNumber()
{
	return SequenceNumber;
}

uint32_t RTPPacket::GetTimeStamp()
{
	return TimeStamp;
}

int RTPPacket::GetSsrc()
{
	return Ssrc;
}