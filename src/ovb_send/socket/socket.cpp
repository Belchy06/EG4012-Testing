#include <iostream>

#include "ovb_common/common.h"
#include "ovb_send/socket/socket.h"

#define LogSendSocket "LogSendSocket"

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
		LOG(LogSendSocket, LOG_SEVERITY_ERROR, "WSAStartup failed with error {}", WSAGetLastError());
		return false;
	}

	// Create a socket
	if ((Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		LOG(LogSendSocket, LOG_SEVERITY_ERROR, "socket failed with error {}", WSAGetLastError());
		return false;
	}

	WSASetIPUserMtu(Sock, 1500);

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
	for (size_t i = 0; i < Packet->GetHeaderSize(); i++)
	{
		Buf[i] = HeaderData[i];
	}

	uint8_t* PayloadData = Packet->GetPayload();
	for (size_t i = 0; i < Packet->GetPayloadSize(); i++)
	{
		Buf[i + HeaderSize] = PayloadData[i];
	}

	// LOG(LogSendSocket, LOG_SEVERITY_DETAILS, "Sending packet {}", Packet->GetSequenceNumber());

	if (sendto(Sock, Buf, (int)TotalSize, 0, (struct sockaddr*)&Other, sizeof(Other)) == SOCKET_ERROR)
	{
		LOG(LogSendSocket, LOG_SEVERITY_ERROR, "sendto failed with error {}", WSAGetLastError());
		return false;
	}

	return true;
}

#undef LogSendSocket