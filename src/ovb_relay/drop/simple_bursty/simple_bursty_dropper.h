#pragma once

#include "ovb_relay/drop/dropper.h"

class SimpleBurstyDropper : public Dropper
{
public:
	SimpleBurstyDropper(DropSettings InOptions);

	virtual bool Drop() override;

private:
	// https://ieeexplore-ieee-org.elibrary.jcu.edu.au/document/6769369
	typedef enum
	{
		GOOD,
		BAD
	} State;

	State CurrentState;
};