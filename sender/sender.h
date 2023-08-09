#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <fstream>

#include "encoder_factory.h"
#include "encoder_callback.h"
#include "packetizer/packetizer.h"
#include "y4m_reader.h"

// Sender should own the socket and the encoder
class Sender : public IEncodeCompleteCallback
{
public:
	Sender();
	~Sender();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void Test();

	// EncodeCompleteCallback interface
	virtual void OnEncodeComplete(const uint8_t* Data, size_t Size) override;

private:
	struct
	{
		ECodec		 Codec;
		std::string	 File;
		ELogSeverity LogLevel = ELogSeverity::SEVERITY_LOG;
	} Settings;

private:
	std::istream*	InputStream;
	std::ifstream	FileStream;
	std::streamsize FileSize;
	std::streamoff	StartSkip;
	std::streamoff	PictureSkip;
	PictureFormat	PicFormat;

	std::shared_ptr<Encoder>	WrappedEncoder;
	std::shared_ptr<Packetizer> Packetizer;
};