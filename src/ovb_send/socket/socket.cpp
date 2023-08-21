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

bool Socket::Send(RTPPacket* Packet)
{
	// Transmit
	const size_t TotalSize = Packet->GetHeaderSize() + Packet->GetPayloadSize();
	char*		 Buf = new char[TotalSize];

	uint8_t* HeaderData = Packet->GetHeader();
	size_t	 HeaderSize = Packet->GetHeaderSize();
	for (int i = 0; i < Packet->GetHeaderSize(); i++)
	{
		Buf[i] = HeaderData[i];
	}

	uint8_t* PayloadData = Packet->GetPayload();
	for (int i = 0; i < Packet->GetPayloadSize(); i++)
	{
		Buf[i + HeaderSize] = PayloadData[i];
	}

	if (sendto(Sock, Buf, TotalSize, 0, (struct sockaddr*)&Other, sizeof(Other)) == SOCKET_ERROR)
	{
		std::cerr << "sendto failed with error" << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}
