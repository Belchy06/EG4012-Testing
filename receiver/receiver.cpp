#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "decoder_factory.h"
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
		// TODO (belchy06): Print help
	}

	SocketConfig Config;
	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);

		// clang-format off
		if (arg == "-h") {
			// TODO (belchy06): Print help
			std::exit(-1);
        } else if(arg == "-port") {
            Config.Port = atoi(argv[++i]);
        } else if(arg == "-codec") {
            std::string CodecStr(argv[++i]);
            if(CodecStr == "H265") {
                Options.Codec = ECodec::CODEC_H265;
            } else if(CodecStr == "AV1") {
                Options.Codec = ECodec::CODEC_AV1;
            } else if(CodecStr == "XVC") {
                Options.Codec = ECodec::CODEC_XVC;
            } else {
                Options.Codec = ECodec::CODEC_UNDEFINED;
            }
        } else if(arg == "-file") {
            Options.File = std::string(argv[++i]);
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
        }
		// clang-format on
	}

	RtpReceiver->Init(Config);
	RtpReceiver->RegisterRTPPacketListener(this);
}

void Receiver::ValidateArgs()
{
	if (Options.File.empty())
	{
		std::cerr << "Error: Missing input file argument" << std::endl;
		std::exit(-1);
	}

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
	// Single NAL Unit mode -> Each packet payload is a single nal unit
	DecodeResult* Result = WrappedDecoder->Decode(InPacket.GetPayload(), InPacket.GetPayloadSize());
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

	Writer.WriteImageHeader(InImage);
}