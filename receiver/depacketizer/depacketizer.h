#pragma once

#include <memory>
#include <vector>

#include "packet.h"
#include "depacketizer_listener.h"

class Depacketizer
{
public:
	static std::shared_ptr<Depacketizer> Create();

	void HandlePacket(RTPPacket InPacket);
	void RegiseterDepacketizerListener(IDepacketizerListener* InDepacketizerListener);

private:
	Depacketizer();

private:
	int					   prevMarkerVal;
	std::vector<RTPPacket> Packets;

	static std::shared_ptr<Depacketizer> Self;
	IDepacketizerListener*				 DepacketizerListener;
};