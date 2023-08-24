#include <random>

#include "ovb_relay/drop/bursty/bursty_dropper.h"

BurstyDropper::BurstyDropper(DropSettings InOptions)
	: CurrentState(State::GOOD)
{
	Options = InOptions;
}

bool BurstyDropper::Drop()
{
	std::mt19937 Gen;
	// Always use same seed for repeatability
	Gen.seed(Seed);
	// Test if we should change state
	std::bernoulli_distribution Dis(CurrentState == State::GOOD ? Options.P : Options.R);
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