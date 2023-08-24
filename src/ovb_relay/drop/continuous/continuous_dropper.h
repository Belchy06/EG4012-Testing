#pragma once

#include "ovb_relay/drop/dropper.h"

class ContinuousDropper : public Dropper
{
public:
	ContinuousDropper(DropSettings InOptions);

	virtual bool Drop() override;
};