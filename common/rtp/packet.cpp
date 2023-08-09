#include "packet.h"

RTPPacket::RTPPacket(int InPType, int InFrameNB, int InTime, const uint8_t* InData, int InDataLength)
{
	// Constant header fields:
	Version = 2;
	Padding = 0;
	Extension = 0;
	CC = 0;
	Marker = 0;
	Ssrc = 0;
	// Variable header fields:
	SequenceNumber = InFrameNB;
	TimeStamp = InTime;
	PayloadType = InPType;

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

	// Construct payload
	PayloadSize = InDataLength;
	Payload = new uint8_t[InDataLength];

	for (int i = 0; i < InDataLength; i++)
	{
		Payload[i] = InData[i];
	}
}

RTPPacket::RTPPacket(uint8_t* InData, int InDataLength)
{
	// fill default fields:
	Version = 2;
	Padding = 0;
	Extension = 0;
	CC = 0;
	Marker = 0;
	Ssrc = 0;

	// check if total packet size is lower than the header size
	if (InDataLength >= HEADER_SIZE)
	{
		// get the header bitsream:
		Header = new uint8_t[HEADER_SIZE];
		for (int i = 0; i < HEADER_SIZE; i++)
			Header[i] = InData[i];

		// get the payload bitstream:
		PayloadSize = InDataLength - HEADER_SIZE;
		Payload = new uint8_t[PayloadSize];
		for (int i = HEADER_SIZE; i < InDataLength; i++)
			Payload[i - HEADER_SIZE] = InData[i];

		// interpret the changing fields of the header:
		PayloadType = Header[1] & 127;
		SequenceNumber = (uint32_t)Header[3] + 256 * (uint32_t)Header[2];
		TimeStamp = (uint32_t)Header[7] + 256 * (uint32_t)Header[6] + 65536 * (uint32_t)Header[5] + 16777216 * (uint32_t)Header[4];
	}
}

uint8_t* RTPPacket::GetHeader()
{
	return Header;
}

int RTPPacket::GetHeaderSize()
{
	return HEADER_SIZE;
}

uint8_t* RTPPacket::GetPayload()
{
	return Payload;
}

int RTPPacket::GetPayloadSize()
{
	return PayloadSize;
}