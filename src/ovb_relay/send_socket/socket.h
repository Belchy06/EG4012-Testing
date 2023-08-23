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

class SendSocket
{
public:
	static std::shared_ptr<SendSocket> Create();
	~SendSocket();

	bool Init(SocketConfig InConfig);
	bool Send(const uint8_t* InData, size_t InSize);

private:
	SendSocket();

private:
	static std::shared_ptr<SendSocket> Self;
	SocketConfig					   Config;
	SOCKET							   Sock;
	struct sockaddr_in				   Other;
};

#undef _WINSOCK_DEPRECATED_NO_WARNINGS