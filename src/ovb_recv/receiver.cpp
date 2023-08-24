#include <filesystem>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "ovb_recv/decoders/decoder_factory.h"
#include "ovb_recv/depacketizer/depacketizer_factory.h"
#include "receiver.h"

Receiver::Receiver()
	: RtpReceiver(RTPReceiver::Create())
	, Writer(nullptr)
{
}

void Receiver::ParseArgs(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		std::cerr << "Error: No args specified" << std::endl;
		PrintHelp();
		std::exit(1);
	}

	SocketConfig Config;
	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);

		// clang-format off
		if(argc - 1 == i) {
            std::cerr << "Error: Missing argument value: " << arg << std::endl;
            PrintHelp();
			std::exit(1);
        } else if(arg == "-h") {
			PrintHelp();
			std::exit(1);
        } else if(arg == "--port") {
            Config.Port = atoi(argv[++i]);
        } else if(arg == "--file") {
            Options.File = std::string(argv[++i]);
        } else if(arg == "--codec") {
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
        } else if(arg == "--log-level") {
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
                std::cerr << "Warning: Unknown log level " << LevelStr << "\n" << "Warning: Default to SEVERITY_INFO" << std::endl;
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            }
        } else {
            std::cerr << "Error: Unknown argument: " << arg << std::endl;
            PrintHelp();
            std::exit(1);
        }
		// clang-format on
	}

	RtpReceiver->Init(Config);
	RtpReceiver->RegisterRTPPacketListener(this);

	Depacketizer = DepacketizerFactory::Create(Options.Codec);
	Depacketizer->RegiseterDepacketizerListener(this);
}

void Receiver::ValidateArgs()
{
	if (Options.File.empty())
	{
		std::cerr << "Error: Missing input file argument" << std::endl;
		std::exit(-1);
	}

	std::filesystem::remove(Options.File);
	FileStream.open(Options.File, std::ios_base::binary);
	if (!FileStream)
	{
		std::cerr << "Error: Failed to open file" << Options.File << std::endl;
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
		std::cerr << "Error: Initializing config" << std::endl;
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
		std::cerr << "Error: Decoding \"" << Result->Error() << "\"" << std::endl;
		// std::exit(-1);
	}
}

void Receiver::OnDecodeComplete(DecodedImage InImage)
{
	std::cout << "====================" << std::endl;
	std::cout << "  OnDecodeComplete  " << std::endl;
	std::cout << "====================" << std::endl;
	std::cout << "Size: " << InImage.Size << std::endl;

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