#pragma once

#include "ovb_relay/settings.h"
#include "ovb_relay/send_socket/socket.h"
#include "ovb_relay/recv_socket/socket.h"
#include "ovb_relay/recv_socket/socket_listener.h"

class Relay : public ISocketListener
{
public:
	Relay();
	~Relay();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void PrintSettings();
	void Run();

private:
	void PrintHelp();

	// ISocketListener interface
	virtual void OnPacketReceived(const uint8_t* InData, size_t InSize) override;

private:
	RelaySettings Options;

	std::shared_ptr<RecvSocket> RecvSock;
	std::shared_ptr<SendSocket> SendSock;
};