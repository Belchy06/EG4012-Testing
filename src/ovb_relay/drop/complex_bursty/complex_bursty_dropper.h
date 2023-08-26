#pragma once

#include "ovb_relay/drop/dropper.h"

class ComplexBurstyDropper : public Dropper
{
public:
	ComplexBurstyDropper(DropSettings InOptions);

	virtual bool Drop() override;

private:
	// https://ieeexplore-ieee-org.elibrary.jcu.edu.au/document/6906364
	typedef enum
	{
		GOOD1,
		GOOD2,
		GOOD3,
		GOOD4,
		BAD
	} State;

	State CurrentState;
};