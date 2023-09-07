#pragma once

#include "ovb_relay/settings.h"
#include "ovb_relay/send_socket/socket.h"
#include "ovb_relay/recv_socket/socket.h"
#include "ovb_relay/recv_socket/socket_listener.h"
#include "ovb_relay/drop/dropper.h"
#include "ovb_relay/tamper/tamperer.h"

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
	uint64_t	  NumPacketsDropped;
	RelaySettings Options;

	std::shared_ptr<RecvSocket> RecvSock;
	std::shared_ptr<SendSocket> SendSock;

	std::shared_ptr<Dropper>  Drop;
	std::shared_ptr<Tamperer> Tamper;

	uint64_t PacketId;
};