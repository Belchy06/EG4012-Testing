#include "receiver.h"

Receiver::Receiver()
	: RtpReceiver(RTPReceiver::Create())
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
}

void Receiver::Run()
{
	RtpReceiver->Receive();
}

void Receiver::OnPacketReceived(RTPPacket InPacket)
{
}