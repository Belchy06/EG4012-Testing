#pragma once

#include <memory>

#include "packet.h"
#include "depacketizer_listener.h"

class Depacketizer
{
public:
	static std::shared_ptr<Depacketizer> Create();

	void HandlePacket(RTPPacket InPacket);
	void RegisterRTPPacketListener(IDepacketizerListener* InDepacketizerListener);

private:
	Depacketizer();

private:
	int prevMarkerVal;

	static std::shared_ptr<Depacketizer> Self;
	IDepacketizerListener*				 DepacketizerListener;
};