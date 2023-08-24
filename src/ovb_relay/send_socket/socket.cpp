#include <iostream>

#include "ovb_common/common.h"
#include "ovb_relay/send_socket/socket.h"

#define LogSendSocket "LogSendSocket"

std::shared_ptr<SendSocket> SendSocket::Self = nullptr;

std::shared_ptr<SendSocket> SendSocket::Create()
{
	if (Self == nullptr)
	{
		std::shared_ptr<SendSocket> Temp(new SendSocket());
		Self = Temp;
	}
	return Self;
}

SendSocket::SendSocket()
{
}

SendSocket::~SendSocket()
{
	closesocket(Sock);
	WSACleanup();
}

bool SendSocket::Init(SocketConfig InConfig)
{
	Config = InConfig;

	// Initialize Winsock
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		LOG(LogSendSocket, LOG_SEVERITY_ERROR, "WSAStartup failed with error: {}", WSAGetLastError());
		return false;
	}

	// Create a socket
	if ((Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		LOG(LogSendSocket, LOG_SEVERITY_ERROR, "socket failed with error: {}", WSAGetLastError());
		return false;
	}

	// setup address structure
	ZeroMemory(&Other, sizeof(Other));
	Other.sin_family = AF_INET;
	Other.sin_port = htons(Config.Port);
	Other.sin_addr.S_un.S_addr = inet_addr(Config.IP.c_str());

	return true;
}

bool SendSocket::Send(const uint8_t* InData, size_t InSize)
{
	// Transmit
	if (sendto(Sock, reinterpret_cast<const char*>(InData), (int)InSize, 0, (struct sockaddr*)&Other, sizeof(Other)) == SOCKET_ERROR)
	{
		LOG(LogSendSocket, LOG_SEVERITY_ERROR, "sendto failed with error: {}", WSAGetLastError());
		return false;
	}

	return true;
}

#undef LogSendSocket