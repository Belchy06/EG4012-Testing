#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include "common.h"
#include "encoder.h"
#include "encoder_factory.h"

#include "y4m_reader.h"
#include <fstream>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_BUFFER_LENGTH 2048
#define DEFAULT_PORT 27015

// Sender should own the socket and the encoder
class Sender
{
public:
	Sender();
	~Sender();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void Test();

private:
	struct
	{
		int			Port;
		ECodec		Codec;
		std::string File;

		int MTUSize;
	} Settings;

private:
	std::istream*	InputStream;
	std::ifstream	FileStream;
	std::streamsize FileSize;
	std::streamoff	StartSkip;
	std::streamoff	PictureSkip;
	PictureFormat	PictureFormat;

	Encoder* WrappedEncoder;
};