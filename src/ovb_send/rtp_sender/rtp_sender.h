#pragma once

#include <memory>
#include <vector>

#include "ovb_common/rtp/packet.h"
#include "ovb_send/socket/socket.h"
#include "ovb_common/socket/socket_config.h"

class RTPSender
{
public:
	static std::shared_ptr<RTPSender> Create();

	bool Init(SocketConfig InConfig);
	bool Send(std::vector<RTPPacket> InPackets);

private:
	RTPSender();

private:
	static std::shared_ptr<RTPSender> Self;
	std::shared_ptr<Socket>			  Sock;
};