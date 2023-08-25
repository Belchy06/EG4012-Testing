#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <fstream>

#include "ovb_send/encoders/encoder_factory.h"
#include "ovb_send/encoders/encoder_callback.h"
#include "ovb_send/packetizer/packetizer_factory.h"
#include "ovb_send/rtp_sender/rtp_sender.h"
#include "ovb_common/settings.h"
#include "ovb_send/y4m_reader.h"

class SenderSettings : public Settings
{
public:
	std::string IP;
};

// Sender should own the socket and the encoder
class Sender : public IEncodeCompleteCallback
{
public:
	Sender();
	~Sender();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void PrintSettings();
	void Run();

private:
	void PrintHelp();
	bool ReadNextPicture(std::istream* InStream, std::vector<uint8_t>& OutPictureBytes);

	// EncodeCompleteCallback interface
	virtual void OnEncodeComplete(uint8_t* InData, size_t InSize) override;

private:
	std::istream*  InputStream;
	std::ifstream  FileStream;
	std::streamoff PictureSkip;
	SenderSettings Options;
	EncoderConfig  Config;

	std::shared_ptr<Encoder>	WrappedEncoder;
	std::shared_ptr<Packetizer> Packetizer;
	std::shared_ptr<RTPSender>	RtpSender;
};