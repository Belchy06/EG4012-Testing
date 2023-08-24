#pragma once

#include <memory>

#include "ovb_relay/drop/dropper.h"

class DropperFactory
{
public:
	static std::shared_ptr<Dropper> Create(EDropType InDropType, DropSettings InOptions);
};