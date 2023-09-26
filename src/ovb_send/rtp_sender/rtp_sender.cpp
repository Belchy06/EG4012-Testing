#include <thread> // std::this_thread::sleep_for
#include <chrono> // std::chrono::seconds

#include "ovb_common/common.h"
#include "rtp_sender.h"

#define LogRtpSender "LogRtpSender"

std::shared_ptr<RTPSender> RTPSender::Self = nullptr;

std::shared_ptr<RTPSender> RTPSender::Create()
{
	if (Self == nullptr)
	{
		std::shared_ptr<RTPSender> Temp(new RTPSender());
		Self = Temp;
	}
	return Self;
}

RTPSender::RTPSender()
	: Sock(Socket::Create())
{
}

bool RTPSender::Init(SocketConfig InConfig)
{
	return Sock->Init(InConfig);
}

bool RTPSender::Send(std::vector<RTPPacket> InPackets)
{
	// TODO (belchy06): Send algorithm (leaky bucket, pacing, etc);
	bool bSuccess = true;
	for (RTPPacket Packet : InPackets)
	{
		LOG(LogRtpSender, LOG_SEVERITY_DETAILS, "Sending packet {}", Packet.GetSequenceNumber());
		bSuccess &= Sock->Send(&Packet);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	return bSuccess;
}

#undef LogRtpSender