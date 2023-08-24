#pragma once

#include "ovb_relay/settings.h"

class Dropper
{
public:
	virtual bool Drop() = 0;

protected:
	DropSettings Options;
	uint16_t	 Seed;
};