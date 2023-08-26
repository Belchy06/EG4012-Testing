#include "ovb_common/common.h"

#include "ovb_relay/drop/complex_bursty/complex_bursty_dropper.h"

ComplexBurstyDropper::ComplexBurstyDropper(DropSettings InOptions)
	: CurrentState(State::GOOD1)
{
	Options = InOptions;
	Gen.seed(InOptions.Seed);
}

bool ComplexBurstyDropper::Drop()
{
	// Test if we should change state
	float TransitionChance = 0;
	switch (CurrentState)
	{
		case GOOD1:
			TransitionChance = Options.P1;
			break;
		case GOOD2:
			TransitionChance = Options.P2;
			break;
		case GOOD3:
			TransitionChance = Options.P3;
			break;
		case GOOD4:
			TransitionChance = Options.P4;
			break;
		case BAD:
			TransitionChance = Options.Q;
			break;
	}

	std::bernoulli_distribution DisBad(TransitionChance);
	bool						bTransition = DisBad(Gen);

	if (bTransition)
	{
		switch (CurrentState)
		{
			case GOOD1:
			case GOOD2:
			case GOOD3:
			case GOOD4:
				CurrentState = BAD;
				break;
			case BAD:
				CurrentState = GOOD1;
				break;
			default:
				unimplemented();
		};
	}
	else
	{
		switch (CurrentState)
		{
			case GOOD1:
				CurrentState = GOOD2;
				break;
			case GOOD2:
				CurrentState = GOOD3;
				break;
			case GOOD3:
				CurrentState = GOOD4;
				break;
			case GOOD4:
				CurrentState = GOOD1;
				break;
			case BAD:
				CurrentState = BAD;
				break;

			default:
				unimplemented();
		};
	}

	std::uniform_real_distribution<float> UDis(0, 1);
	// Use drop percentage based on updated state
	float DropChance = 0;
	switch (CurrentState)
	{
		case GOOD1:
			DropChance = Options.DropChanceGood;
			break;
		case GOOD2:
			DropChance = Options.DropChanceGood;
			break;
		case GOOD3:
			DropChance = Options.DropChanceGood;
			break;
		case GOOD4:
			DropChance = Options.DropChanceGood;
			break;
		case BAD:
			DropChance = Options.DropChanceBad;
			break;
	}
	return UDis(Gen) < DropChance;
}