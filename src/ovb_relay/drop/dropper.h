#pragma once

#include <random>

#include "ovb_relay/settings.h"

class Dropper
{
public:
	virtual bool Drop() = 0;

protected:
	DropSettings Options;
	std::mt19937 Gen;
};