#include <iostream>
#include <string>
#include <sstream>

#include "ovb_common/socket/socket_config.h"
#include "ovb_relay/relay.h"

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
}

Relay::~Relay()
{
}

void Relay::ParseArgs(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		std::cerr << "Error: No args specified" << std::endl;
		PrintHelp();
		std::exit(1);
	}

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
		} else if(arg == "--send-ip") {
            std::stringstream(argv[++i]) >> Options.SendIP;
        } else if(arg == "--send-port") {
            std::stringstream(argv[++i]) >> Options.SendPort;
        } else if(arg == "--recv-port") {
            std::stringstream(argv[++i]) >> Options.RecvPort;
        } else if(arg == "--drop-chance") {
            std::stringstream(argv[++i]) >> Options.DropChance;
        } else if(arg == "--tamper-chance") {
            std::stringstream(argv[++i]) >> Options.TamperChance;
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
        } else if(arg == "--drop-type") {
            std::string LossStr(argv[++i]);
            if(LossStr == "continuous") {
                Options.DropType = EDropType::LOSS_CONTINUOUS;
            } else if(LossStr == "bursty") {
                Options.DropType = EDropType::LOSS_BURSTY;
            } else {
                std::cerr << "Warning: Unknown loss type " << LossStr << "\n" << "Warning: Default to LOSS_BURSTY" << std::endl;
                Options.DropType = EDropType::LOSS_BURSTY;
            }
        } else {
            std::cerr << "Error: Unknown argument: " << arg << std::endl;
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
		std::cerr << "Error: Missing \"--send-ip\" argument" << std::endl;
		PrintHelp();
		std::exit(-1);
	}

	if (Options.SendPort == 0)
	{
		std::cerr << "Error: Missing \"--send-port\" argument" << std::endl;
		PrintHelp();
		std::exit(-1);
	}

	if (Options.RecvPort == 0)
	{
		std::cerr << "Error: Missing \"--recv-port\" argument" << std::endl;
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

	Drop = Dropper::Create(Options.DropChance, Options.DropType, DropperConfig);
	Tamper = Tamperer::Create(Options.TamperChance);
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
    std::cout << "  --log-level: " << SeverityToString(Options.LogLevel) << std::endl;
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
	std::cout << "Received packet " << PacketId << std::endl;
	bool bDrop = false;
	if (Drop)
	{
		bDrop = Drop->Drop();
	}

	if (bDrop)
	{
		// Drop this packet
		std::cout << "[WARNING] Dropping packet " << PacketId << std::endl;
		PacketId++;
		return;
	}

	uint8_t* Data = new uint8_t[InSize];
	if (Tamper)
	{
		Tamper->Tamper(const_cast<uint8_t*>(InData), InSize, &Data);
	}

	if (SendSock)
	{
		SendSock->Send(Data, InSize);
	}

	PacketId++;
}