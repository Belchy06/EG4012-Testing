#include <filesystem>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "decoder_factory.h"
#include "receiver.h"

Receiver::Receiver()
	: RtpReceiver(RTPReceiver::Create())
	, Depacketizer(Depacketizer::Create())
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
        } else if(arg == "-port") {
            Config.Port = atoi(argv[++i]);
        } else if(arg == "-file") {
            Options.File = std::string(argv[++i]);
        } else if(arg == "-codec") {
            std::string CodecStr(argv[++i]);
            if(CodecStr == "H265") {
                Options.Codec = ECodec::CODEC_H265;
            } else if(CodecStr == "XVC") {
                Options.Codec = ECodec::CODEC_XVC;
            } else if(CodecStr == "OVC") {
                Options.Codec = ECodec::CODEC_OVC;
            } else {
                Options.Codec = ECodec::CODEC_UNDEFINED;
            }
        } else if(arg == "-loglevel") {
            std::string LevelStr(argv[++i]);
            if(LevelStr == "log") {
                Options.LogLevel = ELogSeverity::SEVERITY_LOG;
            } else if(LevelStr == "verbose") {
                Options.LogLevel = ELogSeverity::SEVERITY_VERBOSE;
            } else if(LevelStr == "veryverbose") {
                Options.LogLevel = ELogSeverity::SEVERITY_VERY_VERBOSE;
            } else {
                Options.LogLevel = ELogSeverity::SEVERITY_NONE;
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

void Receiver::OnNALReceived(const uint8_t* InData, size_t InSize)
{
	DecodeResult* Result = WrappedDecoder->Decode(InData, InSize);
	if (!Result->IsSuccess())
	{
		std::cerr << "Error: Decoding \"" << Result->Error() << "\"" << std::endl;
		std::exit(-1);
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
	std::cout << "  -file <string> [Optional parameters]" << std::endl << std::endl;
    std::cout << "Optional parameters:" << std::endl;
    std::cout << "  -port <int> (default: 8888)" << std::endl;
    std::cout << "  -loglevel <string> " << std::endl;
    std::cout << "      \"log\"        " << std::endl;
    std::cout << "      \"verbose\"    " << std::endl;
    std::cout << "      \"veryverbose\"" << std::endl;
    std::cout << "  -codec <string>    " << std::endl;
    std::cout << "      \"H265\"       " << std::endl;
    std::cout << "      \"XVC\"        " << std::endl;
    std::cout << "      \"OVC\"        " << std::endl;
	// clang-format on
}