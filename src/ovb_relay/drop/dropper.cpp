#include <random>

#include "ovb_relay/drop/dropper.h"

Dropper::Dropper(float InDropChance)
	: DropChance(InDropChance)
{
}

bool Dropper::Drop()
{
	std::mt19937 Gen;
	// Always use same seed for repeatability
	Gen.seed(0);
	std::uniform_real_distribution<float> Dis(0, 1);
	return Dis(Gen) < DropChance;
}