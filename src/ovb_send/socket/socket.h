#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <memory>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "ovb_common/rtp/packet.h"
#include "ovb_common/socket/socket_config.h"

class Socket
{
public:
	static std::shared_ptr<Socket> Create();
	~Socket();

	bool Init(SocketConfig InConfig);
	bool Send(RTPPacket* Packet);

private:
	Socket();

private:
	static std::shared_ptr<Socket> Self;
	SocketConfig				   Config;
	SOCKET						   Sock;
	struct sockaddr_in			   Other;
};

#undef _WINSOCK_DEPRECATED_NO_WARNINGS