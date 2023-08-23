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
#include "ovb_relay/recv_socket/socket_listener.h"

class RecvSocket
{
public:
	static std::shared_ptr<RecvSocket> Create();
	~RecvSocket();

	bool Init(SocketConfig InConfig);
	void RegisterSocketListener(ISocketListener* InSocketListener);
	bool Receive();

private:
	RecvSocket();

private:
	static std::shared_ptr<RecvSocket> Self;
	SocketConfig					   Config;
	SOCKET							   Sock;
	struct sockaddr_in				   Other;
	int								   OtherLen;
	ISocketListener*				   SocketListener;
};