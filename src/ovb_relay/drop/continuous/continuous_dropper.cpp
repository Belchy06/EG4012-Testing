#include "ovb_relay/drop/continuous/continuous_dropper.h"

ContinuousDropper::ContinuousDropper(DropSettings InOptions)
{
	Options = InOptions;
	Gen.seed(InOptions.Seed);
}

bool ContinuousDropper::Drop()
{
	std::uniform_real_distribution<float> Dis(0, 1);
	return Dis(Gen) < Options.DropChance;
}