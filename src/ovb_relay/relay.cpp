#include <iostream>
#include <string>
#include <sstream>

#include "ovb_common/common.h"
#include "ovb_common/socket/socket_config.h"
#include "ovb_relay/relay.h"

#define LogRelay "LogRelay"

Relay::Relay()
	: PacketId(0)
{
	Options.SendIP = "";
	Options.SendPort = 0;
	Options.RecvPort = 0;
	Options.DropChance = 0.f;
	Options.TamperChance = 0.f;
	Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
	Options.DropType = EDropType::LOSS_BURSTY;
	Options.Seed = 1337;
}

Relay::~Relay()
{
}

void Relay::ParseArgs(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		LOG(LogRelay, LOG_SEVERITY_ERROR, "No args specified");
		PrintHelp();
		std::exit(1);
	}

	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		std::string Arg(argv[i]);

		// clang-format off
		if(argc - 1 == i) {
			LOG(LogRelay, LOG_SEVERITY_ERROR, "Missing argument value: {}", Arg);
            PrintHelp();
			std::exit(1);
        } else if (Arg == "-h") {
			PrintHelp();
			std::exit(1);
		} else if(Arg == "--send-ip") {
            std::stringstream(argv[++i]) >> Options.SendIP;
        } else if(Arg == "--send-port") {
            std::stringstream(argv[++i]) >> Options.SendPort;
        } else if(Arg == "--recv-port") {
            std::stringstream(argv[++i]) >> Options.RecvPort;
        } else if(Arg == "--drop-chance") {
            std::stringstream(argv[++i]) >> Options.DropChance;
        } else if(Arg == "--tamper-chance") {
            std::stringstream(argv[++i]) >> Options.TamperChance;
        } else if(Arg == "--seed") {
            std::stringstream(argv[++i]) >> Options.Seed;
        }else if(Arg == "--log-level") {
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
				LOG(LogRelay, LOG_SEVERITY_WARNING, "Unknown log level \"{}\". Defaulting to LOG_SEVERITY_INFO", LevelStr);
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            }
        } else if(Arg == "--drop-type") {
            std::string LossStr(argv[++i]);
            if(LossStr == "continuous") {
                Options.DropType = EDropType::LOSS_CONTINUOUS;
            } else if(LossStr == "bursty") {
                Options.DropType = EDropType::LOSS_BURSTY;
            } else {
				LOG(LogRelay, LOG_SEVERITY_WARNING, "Unknown loss type \"{}\". Defaulting to LOSS_BURSTY", LossStr);
                Options.DropType = EDropType::LOSS_BURSTY;
            }
        } else {
			LOG(LogRelay, LOG_SEVERITY_ERROR, "Unknown argument \"{}\"", Arg);
            PrintHelp();
            std::exit(1);
        }
		// clang-format on
	}
}

void Relay::ValidateArgs()
{
	if (Options.SendIP.empty())
	{
		LOG(LogRelay, LOG_SEVERITY_ERROR, "Missing \"--send-ip\" argument");
		PrintHelp();
		std::exit(-1);
	}

	if (Options.SendPort == 0)
	{
		LOG(LogRelay, LOG_SEVERITY_ERROR, "Missing \"--send-port\" argument");
		PrintHelp();
		std::exit(-1);
	}

	if (Options.RecvPort == 0)
	{
		LOG(LogRelay, LOG_SEVERITY_ERROR, "Missing \"--recv-port\" argument");
		PrintHelp();
		std::exit(-1);
	}

	SocketConfig RecvConfig;
	RecvConfig.Port = Options.RecvPort;
	RecvSock = RecvSocket::Create();
	RecvSock->Init(RecvConfig);
	RecvSock->RegisterSocketListener(this);

	SocketConfig SendConfig;
	SendConfig.IP = Options.SendIP;
	SendConfig.Port = Options.SendPort;
	SendSock = SendSocket::Create();
	SendSock->Init(SendConfig);

	DropConfig DropperConfig;
	// Values from https://ieeexplore-ieee-org.elibrary.jcu.edu.au/document/6465309 Table 1
	DropperConfig.DropGood = 0.f;
	DropperConfig.DropBad = 1.f;
	DropperConfig.P = 0.05f;
	DropperConfig.R = 0.5f;

	Drop = Dropper::Create(Options.DropChance, Options.DropType, DropperConfig, Options.Seed);
	Tamper = Tamperer::Create(Options.TamperChance, Options.Seed);

	OvbLogging::Verbosity = Options.LogLevel;
}

void Relay::PrintSettings()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Running Relay:" << std::endl;
	std::cout << "  --send-ip: " << Options.SendIP << std::endl;
	std::cout << "  --send-port: " << Options.SendPort << std::endl;
	std::cout << "  --recv-port: " << Options.RecvPort << std::endl;
    std::cout << "  --drop-chance: " << Options.DropChance << std::endl;
    std::cout << "  --drop-type: " << DropTypeToString(Options.DropType) << std::endl;
    std::cout << "  --tamper-chance: " << Options.TamperChance << std::endl;
    std::cout << "  --log-level: " << "LOG_SEVERITY_" << SeverityToString(Options.LogLevel) << std::endl;
	// clang-format on
}

void Relay::Run()
{
	RecvSock->Receive();
}

void Relay::PrintHelp()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
	std::cout << "  --send-ip <string> --send-port <int> --recv-port <int> [Optional parameters]" << std::endl << std::endl;
    std::cout << "Optional parameters:"  << std::endl;
    std::cout << "  --tamper-chance <float> " << std::endl;
    std::cout << "  --drop-chance   <float> " << std::endl;
    std::cout << "  --drop-type     <string>" << std::endl;
    std::cout << "      \"continuous\"      " << std::endl;
    std::cout << "      \"bursty\"          " << std::endl;
    std::cout << "  --log-level     <string>" << std::endl;
    std::cout << "      \"silent\"          " << std::endl;
    std::cout << "      \"error\"           " << std::endl;
    std::cout << "      \"warning\"         " << std::endl;
    std::cout << "      \"info\"            " << std::endl;
    std::cout << "      \"notice\"          " << std::endl;
    std::cout << "      \"verbose\"         " << std::endl;
    std::cout << "      \"details\"         " << std::endl;
	// clang-format on
}

void Relay::OnPacketReceived(const uint8_t* InData, size_t InSize)
{
	bool bDrop = false;
	if (Drop)
	{
		bDrop = Drop->Drop();
	}

	if (bDrop)
	{
		// Drop this packet
		std::cout << "Dropping packet " << PacketId << std::endl;
		PacketId++;
		return;
	}

	uint8_t* Data = new uint8_t[InSize];
	if (Tamper)
	{
		Tamper->Tamper(const_cast<uint8_t*>(InData), InSize, &Data);
	}
	else
	{
		memcpy(Data, InData, InSize);
	}

	if (SendSock)
	{
		SendSock->Send(Data, InSize);
	}

	std::cout << "Passing packet " << PacketId << std::endl;

	PacketId++;
}

#undef LogRelay