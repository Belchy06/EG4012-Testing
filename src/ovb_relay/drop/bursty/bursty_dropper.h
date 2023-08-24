#pragma once

#include "ovb_relay/drop/dropper.h"

class BurstyDropper : public Dropper
{
public:
	BurstyDropper(DropSettings InOptions);

	virtual bool Drop() override;

private:
	//
	typedef enum
	{
		GOOD,
		BAD
	} State;

	State CurrentState;
};