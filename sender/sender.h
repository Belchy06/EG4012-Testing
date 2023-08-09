#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <fstream>

#include "encoder_factory.h"
#include "encoder_callback.h"
#include "packetizer.h"
#include "rtp_sender.h"
#include "settings.h"
#include "y4m_reader.h"

// Sender should own the socket and the encoder
class Sender : public IEncodeCompleteCallback
{
public:
	Sender();
	~Sender();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void Run();

private:
	// EncodeCompleteCallback interface
	virtual void OnEncodeComplete(const uint8_t* InData, size_t InSize) override;

private:
	std::istream*	InputStream;
	std::ifstream	FileStream;
	std::streamsize FileSize;
	std::streamoff	StartSkip;
	std::streamoff	PictureSkip;
	PictureFormat	PicFormat;
	Settings		Options;

	std::shared_ptr<Encoder>	WrappedEncoder;
	std::shared_ptr<Packetizer> Packetizer;
	std::shared_ptr<RTPSender>	RtpSender;
};