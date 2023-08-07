#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "y4m_reader.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFFER_LENGTH 2048
#define DEFAULT_PORT 27015

enum ECodec
{
	H265,
}

struct SParams
{
	int	   Port;
	int	   PacketSize;
	ECodec Codec;
};

int main(int argc, char* argv[])
{
	SParams Params = {
		DEFAULT_PORT,
		DEFAULT_BUFFER_LENGTH,
		ECodec::H265,
	};

	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		// Colon after a letter indicates that that arg param has a following arg
		char* arg = argv[i];
		// increment by one to skip over the '-' when passing params
		switch (*(arg + 1))
		{
			case 'h':
				// TODO (belchy06): Print help

				return -1;

			case 'p':
				Params.Port = atoi(arg + 3);
				continue;

			case 'c':
				std::string CodecStr(arg + 3);
				if (CodecStr == "H265")
				{
					Params.Codec = ECodec::H265;
				}
				continue;

			default:
				continue;
		}
	}

	// Encode sequence

	// Transmit
	SOCKET			   Socket;
	struct sockaddr_in Server, Si_other;
	int				   Slen, Recvlen;
	char			   Buf[DEFAULT_BUFFER_LENGTH];
	WSADATA			   WsaData;

	Slen = sizeof(Si_other);

	// Initialize Winsock
	printf("Initializing Winsock\n");
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		printf("Initialization failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	// Create a socket
	if ((Socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("socket failed with error: %d", WSAGetLastError());
		return -1;
	}

	// Prepare the sockaddr_in structure
	Server.sin_family = AF_INET;
	Server.sin_addr.s_addr = INADDR_ANY;
	Server.sin_port = htons(DEFAULT_PORT);

	// Bind
	if (bind(Socket, (struct sockaddr*)&Server, sizeof(Server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error: %d", WSAGetLastError());
		return -1;
	}

	// keep listening for data
	while (true)
	{
		printf("Waiting for data...");
		fflush(stdout);

		// clear the buffer by filling null, it might have previously received data
		memset(Buf, '\0', DEFAULT_BUFFER_LENGTH);

		// try to receive some data, this is a blocking call
		if ((Recvlen = recvfrom(Socket, Buf, DEFAULT_BUFFER_LENGTH, 0, (struct sockaddr*)&Si_other, &Slen)) == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d", WSAGetLastError());
			return -1;
		}

		// print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(Si_other.sin_addr), ntohs(Si_other.sin_port));
		printf("Data: %s\n", Buf);

		// now reply the client with the same data
		if (sendto(Socket, Buf, Recvlen, 0, (struct sockaddr*)&Si_other, Slen) == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d", WSAGetLastError());
			return -1;
		}
	}

	closesocket(Socket);
	WSACleanup();
	return 0;
}