#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFFER_LENGTH 512
#define DEFAULT_PORT "27015"

int main()
{
	WSADATA WsaData;
	int		Result;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* AddrInfo = NULL;
	struct addrinfo	 Hints;

	int	 SendResult;
	char RecvBuf[DEFAULT_BUFFER_LENGTH];
	int	 RecvBufLength = DEFAULT_BUFFER_LENGTH;

	// Initialize Winsock
	Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Result != 0)
	{
		printf("WSAStartup failed with error: %d\n", Result);
		return -1;
	}

	ZeroMemory(&Hints, sizeof(Hints));
	Hints.ai_family = AF_INET;
	Hints.ai_socktype = SOCK_STREAM;
	Hints.ai_protocol = IPPROTO_UDP;
	Hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	Result = getaddrinfo(NULL, DEFAULT_PORT, &Hints, &AddrInfo);
	if (Result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", Result);
		WSACleanup();
		return -1;
	}

	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(AddrInfo);
		WSACleanup();
		return -1;
	}

	// Setup the UDP listening socket
	Result = bind(ListenSocket, AddrInfo->ai_addr, (int)AddrInfo->ai_addrlen);
	if (Result == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(AddrInfo);
		closesocket(ListenSocket);
		WSACleanup();
		return -1;
	}

	freeaddrinfo(AddrInfo);

	Result = listen(ListenSocket, SOMAXCONN);
	if (Result == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	do
	{

		Result = recv(ClientSocket, RecvBuf, RecvBufLength, 0);
		if (Result > 0)
		{
			printf("Bytes received: %d\n", Result);

			// Echo the buffer back to the sender
			SendResult = send(ClientSocket, RecvBuf, Result, 0);
			if (SendResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return -1;
			}
			printf("Bytes sent: %d\n", SendResult);
		}
		else if (Result == 0)
		{
			printf("Connection closing...\n");
		}
		else
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return -1;
		}
	}
	while (Result > 0);

	// shutdown the connection since we're done
	Result = shutdown(ClientSocket, SD_SEND);
	if (Result == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return -1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}