#include <random>

#include "ovb_relay/settings.h"
#include "ovb_relay/drop/dropper.h"

std::shared_ptr<Dropper> Dropper::Self = nullptr;

std::shared_ptr<Dropper> Dropper::Create(float InDropChance, EDropType InDropType, DropConfig InConfig)
{
	if (Self == nullptr)
	{
		std::shared_ptr<Dropper> Temp(new Dropper(InDropChance, InDropType));
		Self = Temp;
	}
	return Self;
}

Dropper::Dropper(float InDropChance, EDropType InDropType, DropConfig InConfig)
	: DropChance(InDropChance)
	, DropType(InDropType)
	, Config(InConfig)
	, CurrentState(State::GOOD)
{
	States = { State::GOOD, State::BAD };
}

bool Dropper::Drop()
{
	if (DropType == LOSS_CONTINUOUS)
	{
		std::mt19937 Gen;
		// Always use same seed for repeatability
		Gen.seed(0);
		std::uniform_real_distribution<float> Dis(0, 1);
		return Dis(Gen) < DropChance;
	}
	else
	{
		// TODO (belchy06): Markov model
	}
}