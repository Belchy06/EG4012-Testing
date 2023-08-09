#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "packet.h"
#include "socket_config.h"

class Socket
{
public:
	Socket();
	~Socket();

	bool Init(SocketConfig InConfig);
	bool Send(RTPPacket* Packet);

private:
	SocketConfig	   Config;
	SOCKET			   Sock;
	struct sockaddr_in Other;
};