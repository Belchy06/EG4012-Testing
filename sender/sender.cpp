#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "common.h"
#include "encoder.h"
#include "encoder_config.h"
#include "sender.h"

Sender::Sender()
	: InputStream(nullptr)
	, Packetizer(Packetizer::Create())
	, RTPSender(RTPSender::Create())
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
            // Settings.Port = atoi(argv[++i]);
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
        } else if(arg == "-loglevel") {
            std::string LevelStr(argv[++i]);
            if(LevelStr == "log") {
                Settings.LogLevel = ELogSeverity::SEVERITY_LOG;
            } else if(LevelStr == "verbose") {
                Settings.LogLevel = ELogSeverity::SEVERITY_VERBOSE;
            } else if(LevelStr == "veryverbose") {
                Settings.LogLevel = ELogSeverity::SEVERITY_VERY_VERBOSE;
            } else {
                Settings.LogLevel = ELogSeverity::SEVERITY_NONE;
            }
        }
		// clang-format on
	}

	SocketConfig Config;
	Config.IP = "127.0.0.1";
	Config.Port = 8888;
	RTPSender->Init(Config);

	return;
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
	if (!Reader.Read(PicFormat, StartSkip, PictureSkip))
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
	Config.Format = PicFormat.Format;
	Config.Framerate = PicFormat.Framerate;
	Config.Width = PicFormat.Width;
	Config.Height = PicFormat.Height;
	Config.BitDepth = PicFormat.BitDepth;
	Config.StartSkip = StartSkip;
	Config.PictureSkip = PictureSkip;
	Config.LogLevel = Settings.LogLevel;

	EncodeResult* Result = WrappedEncoder->Init(Config);
	if (!Result->IsSuccess())
	{
		std::cerr << "Error: Initializing config" << std::endl;
		std::exit(-1);
	}

	WrappedEncoder->RegisterEncodeCompleteCallback(this);

	Result = WrappedEncoder->Encode(InputStream);
	if (!Result->IsSuccess())
	{
		std::cerr << "Error: Encoding \"" << Result->Error() << "\"" << std::endl;
		std::exit(-1);
	}
}

void Sender::OnEncodeComplete(const uint8_t* InData, size_t InSize)
{
	std::cout << "====================" << std::endl;
	std::cout << "  OnEncodeComplete  " << std::endl;
	std::cout << "====================" << std::endl;
	std::cout << "Size: " << InSize << std::endl;

	std::vector<RTPPacket> Packets = Packetizer->Packetize(InData, InSize);

	RTPSender->Send(Packets);
}