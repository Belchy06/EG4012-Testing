#include "ovb_relay/drop/simple_bursty/simple_bursty_dropper.h"

SimpleBurstyDropper::SimpleBurstyDropper(DropSettings InOptions)
	: CurrentState(State::GOOD)
{
	Options = InOptions;
	Gen.seed(InOptions.Seed);
}

bool SimpleBurstyDropper::Drop()
{
	// Test if we should change state
	std::bernoulli_distribution Dis(CurrentState == State::GOOD ? Options.P : Options.Q);
	bool						ChangeState = Dis(Gen);
	// Update state if we're changing
	if (ChangeState)
	{
		if (CurrentState == State::GOOD)
		{
			CurrentState = State::BAD;
		}
		else
		{
			CurrentState = State::GOOD;
		}
	}
	std::uniform_real_distribution<float> UDis(0, 1);
	// Use drop percentage based on updated state
	return UDis(Gen) < (CurrentState == State::GOOD ? Options.DropChanceGood : Options.DropChanceBad);
}