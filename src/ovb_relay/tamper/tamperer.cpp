#include "ovb_relay/tamper/tamperer.h"

Tamperer::Tamperer(float InTamperChance)
	: TamperChance(InTamperChance)
{
}

void Tamperer::Tamper(uint8_t* InData, uint8_t** OutData)
{
}
