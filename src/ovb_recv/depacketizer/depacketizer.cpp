#include <iostream>

#include "depacketizer.h"

std::shared_ptr<Depacketizer> Depacketizer::Self = nullptr;

std::shared_ptr<Depacketizer> Depacketizer::Create()
{
	if (Self == nullptr)
	{
		std::shared_ptr<Depacketizer> Temp(new Depacketizer());
		Self = Temp;
	}
	return Self;
}

Depacketizer::Depacketizer()
	: prevMarker(1)
	, prevSequenceNumber(0)
	, prevTimestamp(0)
{
}

void Depacketizer::HandlePacket(RTPPacket InPacket)
{
	// TODO (belchy06): Reordering?

	if (InPacket.GetSequenceNumber() < prevSequenceNumber)
	{
		// A packet has come in late, discard it
		return;
	}

	if (InPacket.GetSequenceNumber() > prevSequenceNumber + 1)
	{
		// A packet has come in with a sequence number higher than expected, notify users of the number of packets missed
		std::cout << __FILE__ << "L" << __LINE__ << " Missed packet(s): " << prevSequenceNumber + 1 << "->" << InPacket.GetSequenceNumber() - 1 << std::endl;
		Packets.clear();
	}

	if (InPacket.GetTimeStamp() > prevTimestamp && prevMarker != 1)
	{
		// A packet has come in with a newer timestamp (indicating a new NAL), but we haven't finished receiving the previous NAL
		Packets.clear();
	}

	Packets.push_back(InPacket);

	bool bFullNal = false;
	if (InPacket.GetMarker())
	{
		bFullNal = true;
	}

	if (bFullNal && DepacketizerListener != nullptr)
	{
		// Combine all the RTP packets in the Packets vector into a single packet
		size_t TotalSize = 0;
		for (RTPPacket Packet : Packets)
		{
			TotalSize += Packet.GetPayloadSize();
		}

		uint8_t* TotalData = new uint8_t[TotalSize];
		size_t	 CopiedSize = 0;
		for (RTPPacket Packet : Packets)
		{
			memcpy(TotalData + CopiedSize, Packet.GetPayload(), Packet.GetPayloadSize());
			CopiedSize += Packet.GetPayloadSize();
		}

		DepacketizerListener->OnNALReceived(TotalData, TotalSize);
		Packets.clear();
	}

	prevMarker = InPacket.GetMarker();
	prevTimestamp = InPacket.GetTimeStamp();
	prevSequenceNumber = InPacket.GetSequenceNumber();
}

void Depacketizer::RegiseterDepacketizerListener(IDepacketizerListener* InDepacketizerListener)
{
	DepacketizerListener = InDepacketizerListener;
}