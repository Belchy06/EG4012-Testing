#pragma once

#include <stdint.h>

class IDepacketizerListener
{
public:
	virtual void OnNALReceived(uint8_t* InData, size_t InSize) = 0;
};