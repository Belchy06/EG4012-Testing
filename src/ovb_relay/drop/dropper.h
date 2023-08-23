#pragma once

class Dropper
{
public:
	Dropper(float InDropChance);

	bool Drop();

private:
	float DropChance;
};