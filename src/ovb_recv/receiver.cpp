#include <filesystem>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "ovb_common/common.h"
#include "ovb_recv/decoders/decoder_factory.h"
#include "ovb_recv/depacketizer/depacketizer_factory.h"
#include "ovb_recv/receiver.h"

#define LogReceiver "LogReceiver"

Receiver::Receiver()
	: RtpReceiver(RTPReceiver::Create())
	, Writer(nullptr)
{
}

void Receiver::ParseArgs(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		LOG(LogReceiver, LOG_SEVERITY_ERROR, "No args specified");
		PrintHelp();
		std::exit(1);
	}

	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		std::string Arg(argv[i]);

		// clang-format off
		if(argc - 1 == i) {
			LOG(LogReceiver, LOG_SEVERITY_ERROR, "Missing argument value");
            PrintHelp();
			std::exit(1);
        } else if(Arg == "-h") {
			PrintHelp();
			std::exit(1);
        } else if(Arg == "--port") {
			std::stringstream(argv[++i]) >> Options.Port;
        } else if(Arg == "--file") {
            std::stringstream(argv[++i]) >> Options.File;
        } else if(Arg == "--codec") {
            std::string CodecStr(argv[++i]);
            if(CodecStr == "VVC") {
                Options.Codec = ECodec::CODEC_VVC;
            } else if(CodecStr == "XVC") {
                Options.Codec = ECodec::CODEC_XVC;
            } else if(CodecStr == "OVC") {
                Options.Codec = ECodec::CODEC_OVC;
            } else {
                Options.Codec = ECodec::CODEC_UNDEFINED;
            }
        } else if(Arg == "--log-level") {
            std::string LevelStr(argv[++i]);
            if(LevelStr == "silent") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            } else if(LevelStr == "error") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_ERROR;
            } else if(LevelStr == "warning") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_WARNING;
            } else if(LevelStr == "info") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            } else if(LevelStr == "notice") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_NOTICE;
            } else if(LevelStr == "verbose") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_VERBOSE;
            } else if(LevelStr == "details") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_DETAILS;
            } else {
				LOG(LogReceiver, LOG_SEVERITY_WARNING, "Unknown log level \"{}\". Default to SEVERITY_INFO", LevelStr);
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            }
        } else {
			LOG(LogReceiver, LOG_SEVERITY_WARNING, "Unknown argument \"{}\"", Arg);
            PrintHelp();
            std::exit(1);
        }
		// clang-format on
	}

	OvbLogging::Verbosity = Options.LogLevel;

	SocketConfig Config;
	Config.Port = Options.Port;
	RtpReceiver->Init(Config);
	RtpReceiver->RegisterRTPPacketListener(this);

	Depacketizer = DepacketizerFactory::Create(Options.Codec);
	Depacketizer->RegiseterDepacketizerListener(this);
}

void Receiver::ValidateArgs()
{
	if (Options.File.empty())
	{
		LOG(LogReceiver, LOG_SEVERITY_ERROR, "Missing input file argument");
		std::exit(-1);
	}

	std::filesystem::remove(Options.File);
	FileStream.open(Options.File, std::ios_base::binary);
	if (!FileStream)
	{
		LOG(LogReceiver, LOG_SEVERITY_ERROR, "Failed to open file {}", Options.File);
		std::exit(-1);
	}
	OutputStream = &FileStream;
	Writer = Y4mWriter(OutputStream);
}

void Receiver::Run()
{
	WrappedDecoder = DecoderFactory::Create(Options.Codec);

	DecoderConfig Config;
	DecodeResult* Result = WrappedDecoder->Init(Config);
	if (!Result->IsSuccess())
	{
		LOG(LogReceiver, LOG_SEVERITY_ERROR, "Error initializing config");
		std::exit(-1);
	}

	WrappedDecoder->RegisterDecodeCompleteCallback(this);

	RtpReceiver->Receive();
}

void Receiver::OnPacketReceived(RTPPacket InPacket)
{
	Depacketizer->HandlePacket(InPacket);
}

void Receiver::OnNALReceived(uint8_t* InData, size_t InSize)
{
	DecodeResult* Result = WrappedDecoder->Decode(InData, InSize);
	if (!Result->IsSuccess())
	{
		LOG(LogReceiver, LOG_SEVERITY_ERROR, "Decoding: \"{}\"", Result->Error());
	}
}

void Receiver::OnDecodeComplete(DecodedImage InImage)
{
	LOG(LogReceiver, LOG_SEVERITY_NOTICE, "OnDecodeComplete: Size {}", InImage.Size);

	Writer.WriteImageHeader(InImage);
	Writer.WriteImage(InImage);
}

void Receiver::PrintHelp()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
	std::cout << "  --file <string> [Optional parameters]" << std::endl << std::endl;
    std::cout << "Optional parameters:" << std::endl;
    std::cout << "  --port <int> (default: 8888)" << std::endl;
    std::cout << "  --loglevel <string> " << std::endl;
    std::cout << "      \"silent\"          " << std::endl;
    std::cout << "      \"error\"           " << std::endl;
    std::cout << "      \"warning\"         " << std::endl;
    std::cout << "      \"info\"            " << std::endl;
    std::cout << "      \"notice\"          " << std::endl;
    std::cout << "      \"verbose\"         " << std::endl;
    std::cout << "      \"details\"         " << std::endl;
    std::cout << "  --codec <string>        " << std::endl;
    std::cout << "      \"VVC\"             " << std::endl;
    std::cout << "      \"XVC\"             " << std::endl;
    std::cout << "      \"OVC\"             " << std::endl;
	// clang-format on
}

void Receiver::PrintSettings()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Running Receiver:" << std::endl;
	std::cout << "  --file: " << Options.File << std::endl;
	std::cout << "  --port: " << Options.Port << std::endl;
    std::cout << "  --codec: " << CodecToString(Options.Codec) << std::endl;
    std::cout << "  --log-level: " << "LOG_SEVERITY_" << SeverityToString(Options.LogLevel) << std::endl;
	// clang-format on
}

#undef LogReceiver