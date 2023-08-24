#include <random>

#include "ovb_relay/tamper/tamperer.h"

std::shared_ptr<Tamperer> Tamperer::Self = nullptr;

std::shared_ptr<Tamperer> Tamperer::Create(TamperSettings InOptions)
{
	if (Self == nullptr)
	{
		std::shared_ptr<Tamperer> Temp(new Tamperer(InOptions));
		Self = Temp;
	}
	return Self;
}

Tamperer::Tamperer(TamperSettings InOptions)
{
	Options = InOptions;
}

void Tamperer::Tamper(uint8_t* InData, size_t InSize, uint8_t** OutData)
{
	std::mt19937 Gen;
	// Always use same seed for repeatability
	Gen.seed(Options.Seed);
	std::uniform_real_distribution<float> Dis(0, 1);

	bool bTamper = Dis(Gen) < Options.TamperChance;

	// Don't tamper with this packet, leave packet in original state
	if (!bTamper)
	{
		memcpy(*OutData, InData, InSize);
		return;
	}

	// TODO (belchy06): Tamper

	memcpy(*OutData, InData, InSize);
}
