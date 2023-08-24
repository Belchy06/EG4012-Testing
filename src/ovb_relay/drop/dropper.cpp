#include <random>

#include "ovb_relay/settings.h"
#include "ovb_relay/drop/dropper.h"

std::shared_ptr<Dropper> Dropper::Self = nullptr;

std::shared_ptr<Dropper> Dropper::Create(float InDropChance, EDropType InDropType, DropConfig InConfig, uint16_t InSeed)
{
	if (Self == nullptr)
	{
		std::shared_ptr<Dropper> Temp(new Dropper(InDropChance, InDropType, InConfig, InSeed));
		Self = Temp;
	}
	return Self;
}

Dropper::Dropper(float InDropChance, EDropType InDropType, DropConfig InConfig, uint16_t InSeed)
	: DropChance(InDropChance)
	, DropType(InDropType)
	, Config(InConfig)
	, CurrentState(State::GOOD)
	, Seed(InSeed)
{
}

bool Dropper::Drop()
{
	if (DropType == LOSS_CONTINUOUS)
	{
		std::mt19937 Gen;
		// Always use same seed for repeatability
		Gen.seed(Seed);
		std::uniform_real_distribution<float> Dis(0, 1);
		return Dis(Gen) < DropChance;
	}
	else
	{
		std::mt19937 Gen;
		// Always use same seed for repeatability
		Gen.seed(Seed);
		// Test if we should change state
		std::bernoulli_distribution Dis(CurrentState == State::GOOD ? Config.P : Config.R);
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
		return UDis(Gen) < (CurrentState == State::GOOD ? Config.DropGood : Config.DropBad);
	}
}