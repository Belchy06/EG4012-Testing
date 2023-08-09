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
	uint8_t* Header = InPacket.GetHeader();
    

	int		 Size = 9999;
	uint8_t* Data = new uint8_t[Size];
	if (DepacketizerListener != nullptr)
	{
		DepacketizerListener->OnNALReceived(Data, Size);
	}
}

void Depacketizer::RegisterRTPPacketListener(IDepacketizerListener* InDepacketizerListener)
{
	DepacketizerListener = InDepacketizerListener;
}