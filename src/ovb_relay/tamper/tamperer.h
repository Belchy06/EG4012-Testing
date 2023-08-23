#pragma once

#include <stdint.h>

class Tamperer
{
public:
	Tamperer(float InTamperChance);

	void Tamper(uint8_t* InData, uint8_t** OutData);

private:
	float TamperChance;
};