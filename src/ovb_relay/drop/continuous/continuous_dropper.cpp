#include <random>

#include "ovb_relay/drop/continuous/continuous_dropper.h"

ContinuousDropper::ContinuousDropper(DropSettings InOptions)
{
	Options = InOptions;
}

bool ContinuousDropper::Drop()
{
	std::mt19937 Gen;
	// Always use same seed for repeatability
	Gen.seed(Seed);
	std::uniform_real_distribution<float> Dis(0, 1);
	return Dis(Gen) < Options.DropChance;
}