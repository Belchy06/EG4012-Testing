#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "ovb_common/common.h"
#include "ovb_send/encoders/encoder.h"
#include "ovb_send/encoders/encoder_config.h"
#include "sender.h"

Sender::Sender()
	: InputStream(nullptr)
	, Packetizer(Packetizer::Create())
	, RtpSender(RTPSender::Create())
{
}

Sender::~Sender()
{
}

void Sender::ParseArgs(int argc, const char* argv[])
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
        } else if (arg == "-h") {
			PrintHelp();
			std::exit(1);
		} else if(arg == "--ip") {
            std::stringstream(argv[++i]) >> Config.IP;
        } else if(arg == "--port") {
            Config.Port = atoi(argv[++i]);
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
        } else if(arg == "--file") {
            Options.File = std::string(argv[++i]);
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

	RtpSender->Init(Config);
}

void Sender::ValidateArgs()
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

	if (Options.Codec == ECodec::CODEC_UNDEFINED)
	{
		std::cerr << "Error: Invalid codec" << std::endl;
		std::exit(-1);
	}
}

void Sender::Run()
{
	do
	{
		std::cout << '\n'
				  << "Press a key to continue...";
	}
	while (std::cin.get() != '\n');

	WrappedEncoder = EncoderFactory::Create(Options.Codec);

	EncoderConfig Config;
	Config.Format = PicFormat.Format;
	Config.Framerate = PicFormat.Framerate;
	Config.Width = PicFormat.Width;
	Config.Height = PicFormat.Height;
	Config.BitDepth = PicFormat.BitDepth;
	Config.StartSkip = StartSkip;
	Config.PictureSkip = PictureSkip;
	Config.LogLevel = Options.LogLevel;

	EncodeResult* Result = WrappedEncoder->Init(Config);
	if (!Result->IsSuccess())
	{
		std::cerr << "Error: Initializing config" << std::endl;
		std::exit(-1);
	}

	WrappedEncoder->RegisterEncodeCompleteCallback(this);

	int PictureSamples = 0;
	if (Config.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME)
	{
		PictureSamples = Config.Width * Config.Height;
	}
	else if (Config.Format == EChromaFormat::CHROMA_FORMAT_420)
	{
		PictureSamples = (3 * (Config.Width * Config.Height)) >> 1;
	}
	else if (Config.Format == EChromaFormat::CHROMA_FORMAT_422)
	{
		PictureSamples = 2 * Config.Width * Config.Height;
	}
	else if (Config.Format == EChromaFormat::CHROMA_FORMAT_444)
	{
		PictureSamples = 3 * Config.Width * Config.Height;
	}

	PictureBytes.resize(Config.BitDepth == 8 ? PictureSamples : (PictureSamples << 1));

	bool bContinue = true;
	while (bContinue)
	{
		bContinue = ReadNextPicture(InputStream, PictureBytes);
		bool bLastPic = !bContinue;

		Result = WrappedEncoder->Encode(PictureBytes, bLastPic);
		if (!Result->IsSuccess())
		{
			std::cerr << "Error: Encoding \"" << Result->Error() << "\"" << std::endl;
			std::exit(-1);
		}
	}
}

void Sender::OnEncodeComplete(const uint8_t* InData, size_t InSize)
{
	// std::cout << "====================" << std::endl;
	// std::cout << "  OnEncodeComplete  " << std::endl;
	// std::cout << "====================" << std::endl;
	// std::cout << "Size: " << InSize << std::endl;

	std::vector<RTPPacket> Packets = Packetizer->Packetize(InData, InSize);

	RtpSender->Send(Packets);
}

bool Sender::ReadNextPicture(std::istream* InStream, std::vector<uint8_t>& OutPictureBytes)
{
	if (PictureSkip > 0)
	{
		InStream->seekg(PictureSkip, std::ifstream::cur);
	}
	InStream->read(reinterpret_cast<char*>(&(OutPictureBytes)[0]), OutPictureBytes.size());
	return InStream->gcount() == static_cast<int>(OutPictureBytes.size());
}

void Sender::PrintHelp()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
	std::cout << "  --file <string> [Optional parameters]" << std::endl << std::endl;
    std::cout << "Optional parameters:" << std::endl;
    std::cout << "  --ip <string>       (default: \"127.0.0.1\")" << std::endl;
    std::cout << "  --port <int>        (default: 8888)" << std::endl;
    std::cout << "  --log-level <string> " << std::endl;
    std::cout << "      \"log\"        " << std::endl;
    std::cout << "      \"verbose\"    " << std::endl;
    std::cout << "      \"veryverbose\"" << std::endl;
    std::cout << "  --codec <string>    " << std::endl;
    std::cout << "      \"VVC\"        " << std::endl;
    std::cout << "      \"XVC\"        " << std::endl;
    std::cout << "      \"OVC\"        " << std::endl;
	// clang-format on
}