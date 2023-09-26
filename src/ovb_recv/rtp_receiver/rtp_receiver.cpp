#include "rtp_receiver.h"

#include "ovb_common/common.h"

#define LogRtpReceiver "LogRtpReceiver"

std::shared_ptr<RTPReceiver> RTPReceiver::Self = nullptr;

std::shared_ptr<RTPReceiver> RTPReceiver::Create()
{
	if (Self == nullptr)
	{
		std::shared_ptr<RTPReceiver> Temp(new RTPReceiver());
		Self = Temp;
	}
	return Self;
}

RTPReceiver::RTPReceiver()
	: Sock(Socket::Create())
{
}

bool RTPReceiver::Init(SocketConfig InConfig)
{
	Sock->RegisterSocketListener(this);
	return Sock->Init(InConfig);
}

bool RTPReceiver::Receive()
{
	return Sock->Receive();
}

void RTPReceiver::OnPacketReceived(const uint8_t* InData, size_t InSize)
{
	RTPPacket Packet(InData, InSize);

	// LOG(LogRtpReceiver, LOG_SEVERITY_DETAILS, "received packet {}", Packet.GetSequenceNumber());

	if (RTPPacketListener != nullptr)
	{
		RTPPacketListener->OnPacketReceived(Packet);
	}
}

void RTPReceiver::RegisterRTPPacketListener(IRTPPacketListener* InRTPPacketListener)
{
	RTPPacketListener = InRTPPacketListener;
}

#undef LogRtpReceiver