#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <memory>

#include "packet.h"
#include "rtp_receiver.h"
#include "rtp_receiver_listener.h"
#include "settings.h"

class Receiver : public IRTPPacketListener
{
public:
	Receiver();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void Run();

private:
	// IRTPPacketListener interface
	virtual void OnPacketReceived(RTPPacket InPacket) override;

private:
	Settings Options;

	std::shared_ptr<RTPReceiver> RtpReceiver;
};