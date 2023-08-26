#include <iostream>
#include <string>
#include <sstream>

#include "ovb_common/common.h"
#include "ovb_common/socket/socket_config.h"
#include "ovb_relay/relay.h"
#include "ovb_relay/drop/dropper_factory.h"

#define LogRelay "LogRelay"

Relay::Relay()
	: PacketId(0)
{
	Options.SendIP = "";
	Options.SendPort = 0;
	Options.RecvPort = 0;
	Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
	Options.DropType = EDropType::LOSS_COMPLEX_BURSTY;
	Options.DropOptions.Seed = Options.TamperOptions.Seed = 1337;

	// Values from https://ieeexplore-ieee-org.elibrary.jcu.edu.au/document/6465309 Table 1
	Options.DropOptions.DropChanceGood = 0.f;
	Options.DropOptions.DropChanceBad = 0.9998f;
	Options.DropOptions.P1 = 0.0031f;
	Options.DropOptions.P2 = 0.0034f;
	Options.DropOptions.P3 = 0.0036f;
	Options.DropOptions.P4 = 0.1004f;
	Options.DropOptions.Q = 0.2786f;
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
        } else if(Arg == "--seed") {
            uint16_t Seed;
            std::stringstream(argv[++i]) >> Seed;
            Options.DropOptions.Seed = Seed;
            Options.TamperOptions.Seed = Seed;
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
				LOG(LogRelay, LOG_SEVERITY_WARNING, "Unknown log level \"{}\". Defaulting to LOG_SEVERITY_INFO", LevelStr);
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            }
        } else if(Arg == "--drop-type") {
            std::string LossStr(argv[++i]);
            if(LossStr == "continuous") {
                Options.DropType = EDropType::LOSS_CONTINUOUS;
            } else if(LossStr == "simple_bursty") {
                Options.DropType = EDropType::LOSS_SIMPLE_BURSTY;
            } else if(LossStr == "complex_bursty") {
                Options.DropType = EDropType::LOSS_COMPLEX_BURSTY;
            } else {
				LOG(LogRelay, LOG_SEVERITY_WARNING, "Unknown loss type \"{}\". Defaulting to LOSS_COMPLEX_BURSTY", LossStr);
                Options.DropType = EDropType::LOSS_COMPLEX_BURSTY;
            }
        } else if(Arg == "--drop-config") {
            std::string Config(argv[++i]);
            std::istringstream Stream(Config);
            std::string Option;
            while(std::getline(Stream, Option, ','))
            {
                std::string Key = Option.substr(0, Option.find("="));
                std::string Value = Option.substr(Option.find("=") + 1);
                if(Key == "--hg") {
                    std::stringstream(Value) >> Options.DropOptions.DropChanceGood;
                } else if(Key == "--hb") {
                    std::stringstream(Value) >> Options.DropOptions.DropChanceBad;
                } else if(Key == "--p1") {
                    std::stringstream(Value) >> Options.DropOptions.P1;
                } else if(Key == "--p2") {
                    std::stringstream(Value) >> Options.DropOptions.P2;
                } else if(Key == "--p3") {
                    std::stringstream(Value) >> Options.DropOptions.P3;
                } else if(Key == "--p4") {
                    std::stringstream(Value) >> Options.DropOptions.P4;
                } else if(Key == "--q") {
                    std::stringstream(Value) >> Options.DropOptions.Q;
                } else if(Key == "--p") {
                    std::stringstream(Value) >> Options.DropOptions.P;
                } else if(Key == "--prob") {
                    std::stringstream(Value) >> Options.DropOptions.DropChance;
                } else {
                    LOG(LogRelay, LOG_SEVERITY_WARNING, "Unknown drop config option \"{}\"", Key);
                }
            }
        } else if(Arg == "--tamper-config") {
            std::string Config(argv[++i]);
            std::istringstream Stream(Config);
            std::string Option;
            while(std::getline(Stream, Option, ','))
            {
                std::string Key = Option.substr(0, Option.find("="));
                std::string Value = Option.substr(0, Option.find("="));
                if(Key == "--prob") {
                    std::stringstream(Value) >> Options.TamperOptions.TamperChance;
                } else {
                    LOG(LogRelay, LOG_SEVERITY_WARNING, "Unknown tamper config option \"{}\"", Key);
                }
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

	Drop = DropperFactory::Create(Options.DropType, Options.DropOptions);
	Tamper = Tamperer::Create(Options.TamperOptions);

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
    std::cout << "  --drop-type: " << DropTypeToString(Options.DropType) << std::endl;
    std::cout << "  --drop-config: " << std::endl;
    std::cout << "    --seed: " << Options.DropOptions.Seed << std::endl;
    if(Options.DropType == EDropType::LOSS_SIMPLE_BURSTY) {
    std::cout << "    --hg: " << Options.DropOptions.DropChanceGood << std::endl;
    std::cout << "    --hb: " << Options.DropOptions.DropChanceBad << std::endl;
    std::cout << "    --p: " << Options.DropOptions.P << std::endl;
    std::cout << "    --q: " << Options.DropOptions.Q  << std::endl;
    } else if(Options.DropType == EDropType::LOSS_COMPLEX_BURSTY) {
    std::cout << "    --hg: " << Options.DropOptions.DropChanceGood << std::endl;
    std::cout << "    --hb: " << Options.DropOptions.DropChanceBad << std::endl;
    std::cout << "    --p1: " << Options.DropOptions.P1 << std::endl;
    std::cout << "    --p2: " << Options.DropOptions.P2 << std::endl;
    std::cout << "    --p3: " << Options.DropOptions.P3 << std::endl;
    std::cout << "    --p4: " << Options.DropOptions.P4 << std::endl;
    std::cout << "    --q: " << Options.DropOptions.Q  << std::endl;
    } else if(Options.DropType == EDropType::LOSS_CONTINUOUS) {
    std::cout << "    --prob: " << Options.DropOptions.DropChance << std::endl;
    }
    std::cout << "  --tamper-config: " << std::endl;
    std::cout << "    --prob: " << Options.TamperOptions.TamperChance << std::endl;
    std::cout << "    --seed: " << Options.TamperOptions.Seed << std::endl;
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
    std::cout << "  --seed <int>            " << std::endl;
    std::cout << "  --drop-type     <string>" << std::endl;
    std::cout << "      \"continuous\"      " << std::endl;
    std::cout << "      \"bursty\"          " << std::endl;
    std::cout << "  --drop-config   <string>" << std::endl;
    std::cout << "      --hg=<float>,       " << std::endl;
    std::cout << "      --hb=<float>,       " << std::endl;
    std::cout << "      --p1=<float>,        " << std::endl;
    std::cout << "      --p2=<float>,        " << std::endl;
    std::cout << "      --p3=<float>,        " << std::endl;
    std::cout << "      --p4=<float>,        " << std::endl;
    std::cout << "      --q=<float>,        " << std::endl;
    std::cout << "      --prob=<float>      " << std::endl;
    std::cout << "  --tamper-config <string>" << std::endl;
    std::cout << "      --prob=<float>      " << std::endl;
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
		LOG(LogRelay, LOG_SEVERITY_NOTICE, "Dropping packet {}", PacketId);
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

	LOG(LogRelay, LOG_SEVERITY_DETAILS, "Passing packet {}", PacketId);

	PacketId++;
}

#undef LogRelay