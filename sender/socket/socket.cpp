#ifndef DEFAULT_BUFFER_LENGTH
	#define DEFAULT_BUFFER_LENGTH 2048
#endif

#ifndef DEFAULT_PORT
	#define DEFAULT_PORT 27015
#endif

#include "socket.h"

Socket::Socket()
{
}

Socket::~Socket()
{
	closesocket(Socket);
	WSACleanup();
}

Socket::Init()
{
	SOCKET			   Socket;
	struct sockaddr_in Server, Si_other;
	int				   Slen, Recvlen;
	char			   Buf[DEFAULT_BUFFER_LENGTH];
	WSADATA			   WsaData;
}

Socket::Send()
{
	// Transmit

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

	return 0;
}