#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <memory>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "ovb_common/rtp/packet.h"
#include "ovb_common/socket/socket_config.h"
#include "ovb_recv/socket/socket_listener.h"

class Socket
{
public:
	static std::shared_ptr<Socket> Create();
	~Socket();

	bool Init(SocketConfig InConfig);
	void RegisterSocketListener(ISocketListener* InSocketListener);
	bool Receive();

private:
	Socket();

private:
	static std::shared_ptr<Socket> Self;
	SocketConfig				   Config;
	SOCKET						   Sock;
	struct sockaddr_in			   Other;
	int							   OtherLen;
	ISocketListener*			   SocketListener;
};