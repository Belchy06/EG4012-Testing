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
	: prevMarkerVal(1)
{
}

void Depacketizer::HandlePacket(RTPPacket InPacket)
{
	// TODO (belchy06):
	/*
	 * Buffer incoming packet
	 * Check incoming sequence number for out of order packets
	 * On receiving a packet with Mark = 1 => Call listener
	 *
	 */
	Packets.push_back(InPacket);

	bool bFullNal = false;
	if (InPacket.GetMarker())
	{
		bFullNal = true;
	}

	if (bFullNal && DepacketizerListener != nullptr)
	{
		// Combine all the RTP packets in the Packets vector into a single packet
		int TotalSize = 0;
		for (RTPPacket Packet : Packets)
		{
			TotalSize += Packet.GetPayloadSize();
		}

		uint8_t* TotalData = new uint8_t[TotalSize];
		int		 CopiedSize = 0;
		for (RTPPacket Packet : Packets)
		{
			memcpy(TotalData + CopiedSize, Packet.GetPayload(), Packet.GetPayloadSize());
			CopiedSize += Packet.GetPayloadSize();
		}

		DepacketizerListener->OnNALReceived(TotalData, TotalSize);
		Packets.clear();
	}

	prevMarkerVal = InPacket.GetMarker();
}

void Depacketizer::RegiseterDepacketizerListener(IDepacketizerListener* InDepacketizerListener)
{
	DepacketizerListener = InDepacketizerListener;
}