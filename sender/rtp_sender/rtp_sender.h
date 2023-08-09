#pragma once

#include <memory>
#include <vector>

#include "packet.h"
#include "socket.h"
#include "socket_config.h"

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