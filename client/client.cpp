#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#include <cstdio>

#pragma comment(lib, "ws2_32.lib") // Winsock Library

#define DEFAULT_IP "127.0.0.1"	   // ip address of udp server
#define DEFAULT_BUFFER_LENGTH 2048 // Max length of buffer
#define DEFAULT_PORT 27015		   // The port on which to listen for incoming data

int main(int argc, char* argv[])
{
	struct sockaddr_in Si_other;
	SOCKET			   Socket;
	int				   Slen = sizeof(Si_other);
	char			   Buf[DEFAULT_BUFFER_LENGTH];
	char			   Message[DEFAULT_BUFFER_LENGTH];
	WSADATA			   WsaData;

	// Initialise winsock
	printf("Initialising Winsock\n");
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		printf("Initialization failed with error: %d", WSAGetLastError());
		return -1;
	}

	// create socket
	if ((Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket failed with error: %d", WSAGetLastError());
		return -1;
	}

	// setup address structure
	ZeroMemory(&Si_other, sizeof(Si_other));
	Si_other.sin_family = AF_INET;
	Si_other.sin_port = htons(DEFAULT_PORT);
	Si_other.sin_addr.S_un.S_addr = inet_addr(DEFAULT_IP);

	// start communication
	while (true)
	{
		printf("Enter message : ");
		fgets(Message, DEFAULT_BUFFER_LENGTH, stdin);

		// send the message
		if (sendto(Socket, Message, strlen(Message), 0, (struct sockaddr*)&Si_other, Slen) == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d", WSAGetLastError());
			return -1;
		}

		// receive a reply and print it
		// clear the buffer by filling null, it might have previously received data
		memset(Buf, '\0', DEFAULT_BUFFER_LENGTH);
		// try to receive some data, this is a blocking call
		if (recvfrom(Socket, Buf, DEFAULT_BUFFER_LENGTH, 0, (struct sockaddr*)&Si_other, &Slen) == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d", WSAGetLastError());
			return -1;
		}

		puts(Buf);
	}

	closesocket(Socket);
	WSACleanup();

	return 0;
}