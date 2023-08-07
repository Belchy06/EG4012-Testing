

// #include <windows.h>
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #include <iphlpapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "sender.h"
#include "encoder.h"
#include "encoders/config.h"
#include "y4m_reader.h"

Sender::Sender()
	: InputStream(nullptr)
{
}

Sender::~Sender()
{
}

void Sender::ParseArgs(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		// TODO (belchy06): Print help
	}

	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);

		// clang-format off
		if (arg == "-h") {
			// TODO (belchy06): Print help
			std::exit(-1);
		} else if(arg == "-port") {
            Settings.Port = atoi(argv[++i]);
        } else if(arg == "-codec") {
            std::string CodecStr(argv[++i]);
            if(CodecStr == "H265") {
                Settings.Codec = ECodec::CODEC_H265;
            } else if(CodecStr == "AV1") {
                Settings.Codec = ECodec::CODEC_AV1;
            } else if(CodecStr == "XVC") {
                Settings.Codec = ECodec::CODEC_XVC;
            } else {
                Settings.Codec = ECodec::CODEC_UNDEFINED;
            }
        } else if(arg == "-file") {
            Settings.File = std::string(argv[++i]);
        }
		// clang-format on
	}

	return;

	// Encode sequence

	// Transmit
	// SOCKET			   Socket;
	// struct sockaddr_in Server, Si_other;
	// int				   Slen, Recvlen;
	// char			   Buf[DEFAULT_BUFFER_LENGTH];
	// WSADATA			   WsaData;

	// Slen = sizeof(Si_other);

	// // Initialize Winsock
	// printf("Initializing Winsock\n");
	// if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	// {
	// 	printf("Initialization failed with error: %d\n", WSAGetLastError());
	// 	return -1;
	// }

	// // Create a socket
	// if ((Socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	// {
	// 	printf("socket failed with error: %d", WSAGetLastError());
	// 	return -1;
	// }

	// // Prepare the sockaddr_in structure
	// Server.sin_family = AF_INET;
	// Server.sin_addr.s_addr = INADDR_ANY;
	// Server.sin_port = htons(DEFAULT_PORT);

	// // Bind
	// if (bind(Socket, (struct sockaddr*)&Server, sizeof(Server)) == SOCKET_ERROR)
	// {
	// 	printf("Bind failed with error: %d", WSAGetLastError());
	// 	return -1;
	// }

	// // keep listening for data
	// while (true)
	// {
	// 	printf("Waiting for data...");
	// 	fflush(stdout);

	// 	// clear the buffer by filling null, it might have previously received data
	// 	memset(Buf, '\0', DEFAULT_BUFFER_LENGTH);

	// 	// try to receive some data, this is a blocking call
	// 	if ((Recvlen = recvfrom(Socket, Buf, DEFAULT_BUFFER_LENGTH, 0, (struct sockaddr*)&Si_other, &Slen)) == SOCKET_ERROR)
	// 	{
	// 		printf("recvfrom failed with error: %d", WSAGetLastError());
	// 		return -1;
	// 	}

	// 	// print details of the client/peer and the data received
	// 	printf("Received packet from %s:%d\n", inet_ntoa(Si_other.sin_addr), ntohs(Si_other.sin_port));
	// 	printf("Data: %s\n", Buf);

	// 	// now reply the client with the same data
	// 	if (sendto(Socket, Buf, Recvlen, 0, (struct sockaddr*)&Si_other, Slen) == SOCKET_ERROR)
	// 	{
	// 		printf("sendto failed with error: %d", WSAGetLastError());
	// 		return -1;
	// 	}
	// }

	// closesocket(Socket);
	// WSACleanup();
	// return 0;
}

void Sender::ValidateArgs()
{
	if (Settings.File.empty())
	{
		std::cerr << "Error: Missing input file argument" << std::endl;
		std::exit(-1);
	}

	FileStream.open(Settings.File, std::ios_base::binary);
	if (!FileStream)
	{
		std::cerr << "Error: Failed to open file" << Settings.File << std::endl;
		std::exit(-1);
	}
	InputStream = &FileStream;
	InputStream->seekg(0, std::ifstream::end);
	FileSize = InputStream->tellg();
	InputStream->seekg(0, std::ifstream::beg);

	Y4mReader Reader(InputStream);
	if (!Reader.Read(PictureFormat, StartSkip, PictureSkip))
	{
		std::cout << "Reading unsuccessful" << std::endl;
		std::exit(-1);
	}

	if (Settings.Codec == ECodec::CODEC_UNDEFINED)
	{
		std::cerr << "Error: Invalid codec" << std::endl;
		std::exit(-1);
	}
}

void Sender::Test()
{
	WrappedEncoder = EncoderFactory::Create(Settings.Codec);

	EncoderConfig Config;
	Config.Format = PictureFormat.Format;
	Config.Framerate = PictureFormat.Framerate;
	Config.Width = PictureFormat.Width;
	Config.Height = PictureFormat.Height;
	Config.BitDepth = PictureFormat.BitDepth;

	EncodeResult* Res = WrappedEncoder->Init(Config);
	if (!Res->IsSuccess())
	{
		std::cerr << "Error: Initializing config" << std::endl;
		std::exit(-1);
	}

	// WrappedEncoder->Encode();
}