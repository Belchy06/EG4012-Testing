#pragma once

#include <memory>
#include <stdint.h>

class Tamperer
{
public:
	static std::shared_ptr<Tamperer> Create(float InTamperChance, uint16_t InSeed);

	void Tamper(uint8_t* InData, size_t InSize, uint8_t** OutData);

private:
	Tamperer(float InTamperChance, uint16_t InSeed);

private:
	static std::shared_ptr<Tamperer> Self;
	float							 TamperChance;
	uint16_t						 Seed;
};