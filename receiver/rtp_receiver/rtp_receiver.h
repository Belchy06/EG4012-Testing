#pragma once

#include <memory>

#include "socket_listener.h"
#include "packet.h"
#include "rtp_receiver_listener.h"
#include "socket.h"
#include "socket_config.h"

class RTPReceiver : public ISocketListener
{
public:
	static std::shared_ptr<RTPReceiver> Create();

	bool Init(SocketConfig InConfig);
	bool Receive();
	void RegisterRTPPacketListener(IRTPPacketListener* InRTPPacketListener);

	// ISocketListener interface
	virtual void OnPacketReceived(const uint8_t* InData, size_t InSize) override;

private:
	RTPReceiver();

private:
	static std::shared_ptr<RTPReceiver> Self;
	std::shared_ptr<Socket>				Sock;
	IRTPPacketListener*					RTPPacketListener;
};