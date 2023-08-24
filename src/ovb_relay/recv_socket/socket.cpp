#ifndef DEFAULT_BUFFER_LENGTH
	#define DEFAULT_BUFFER_LENGTH 2048
#endif

#include <iostream>

#include "ovb_common/common.h"
#include "socket.h"

#define LogRecvSocket "LogRecvSocket"

std::shared_ptr<RecvSocket> RecvSocket::Self = nullptr;

std::shared_ptr<RecvSocket> RecvSocket::Create()
{
	if (Self == nullptr)
	{
		std::shared_ptr<RecvSocket> Temp(new RecvSocket());
		Self = Temp;
	}
	return Self;
}

RecvSocket::RecvSocket()
{
}

RecvSocket::~RecvSocket()
{
	closesocket(Sock);
	WSACleanup();
}

bool RecvSocket::Init(SocketConfig InConfig)
{
	Config = InConfig;

	// Initialize Winsock
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		LOG(LogRecvSocket, LOG_SEVERITY_ERROR, "WSAStartup failed with error: {}", WSAGetLastError());
		return false;
	}

	// Create a socket
	if ((Sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		LOG(LogRecvSocket, LOG_SEVERITY_ERROR, "socket failed with error: {}", WSAGetLastError());
		return false;
	}

	// Setup address structure
	Other.sin_family = AF_INET;
	Other.sin_addr.s_addr = INADDR_ANY;
	Other.sin_port = htons(Config.Port);
	OtherLen = sizeof(Other);

	// Bind
	if (bind(Sock, (struct sockaddr*)&Other, OtherLen) == SOCKET_ERROR)
	{
		LOG(LogRecvSocket, LOG_SEVERITY_ERROR, "bind failed with error: {}", WSAGetLastError());
		return false;
	}

	return true;
}

bool RecvSocket::Receive()
{
	char* Buf = new char[DEFAULT_BUFFER_LENGTH];
	while (true)
	{
		// clear the buffer by filling null, it might have previously received data
		memset(Buf, '\0', DEFAULT_BUFFER_LENGTH);

		// try to receive some data, this is a blocking call
		int RecvLen;
		if ((RecvLen = recvfrom(Sock, Buf, DEFAULT_BUFFER_LENGTH, 0, (struct sockaddr*)&Other, &OtherLen)) == SOCKET_ERROR)
		{
			LOG(LogRecvSocket, LOG_SEVERITY_ERROR, "recvfrom failed with error: {}", WSAGetLastError());
			return false;
		}

		if (SocketListener != nullptr)
		{
			SocketListener->OnPacketReceived((uint8_t*)Buf, RecvLen);
		}
	}

	return true;
}

void RecvSocket::RegisterSocketListener(ISocketListener* InSocketListener)
{
	SocketListener = InSocketListener;
}

#undef LogRecvSocket