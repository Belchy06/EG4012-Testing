#pragma once

#include <memory>
#include <stdint.h>

#include "ovb_relay/settings.h"

class Tamperer
{
public:
	static std::shared_ptr<Tamperer> Create(TamperSettings InOptions);

	void Tamper(uint8_t* InData, size_t InSize, uint8_t** OutData);

private:
	Tamperer(TamperSettings InOptions);

private:
	static std::shared_ptr<Tamperer> Self;
	TamperSettings					 Options;
};