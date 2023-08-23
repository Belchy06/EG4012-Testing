#pragma once

#include <stdint.h>

class ISocketListener
{
public:
	virtual void OnPacketReceived(const uint8_t* InData, size_t InSize) = 0;
};