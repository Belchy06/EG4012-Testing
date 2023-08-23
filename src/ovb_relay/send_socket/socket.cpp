#include <iostream>

#include "socket.h"

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
		std::cerr << "WSAStartup failed with error" << WSAGetLastError() << std::endl;
		return false;
	}

	// Create a socket
	if ((Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		std::cerr << "socket failed with error" << WSAGetLastError() << std::endl;
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
		std::cerr << "sendto failed with error" << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}
