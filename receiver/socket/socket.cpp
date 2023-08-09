#ifndef DEFAULT_BUFFER_LENGTH
	#define DEFAULT_BUFFER_LENGTH 2048
#endif

#include <iostream>

#include "socket.h"

std::shared_ptr<Socket> Socket::Self = nullptr;

std::shared_ptr<Socket> Socket::Create()
{
	if (Self == nullptr)
	{
		std::shared_ptr<Socket> Temp(new Socket());
		Self = Temp;
	}
	return Self;
}

Socket::Socket()
{
}

Socket::~Socket()
{
	closesocket(Sock);
	WSACleanup();
}

bool Socket::Init(SocketConfig InConfig)
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
	if ((Sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		std::cerr << "socket failed with error" << WSAGetLastError() << std::endl;
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

		std::cerr << "bind failed with error" << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

bool Socket::Receive()
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
			printf("recvfrom failed with error: %d", WSAGetLastError());
			return false;
		}

		if (SocketListener != nullptr)
		{
			SocketListener->OnPacketReceived((uint8_t*)Buf, RecvLen);
		}

		// print details of the client/peer and the data received
		std::cout << "Received packet from " << inet_ntoa(Other.sin_addr) << ":" << ntohs(Other.sin_port) << std::endl;
	}

	return true;
}

void Socket::RegisterSocketListener(ISocketListener* InSocketListener)
{
	SocketListener = InSocketListener;
}