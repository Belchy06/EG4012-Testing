#pragma once

#include <memory>

class DropConfig
{
public:
	// h_g
	float DropGood;
	// h_b
	float DropBad;

	// p
	float P;
	// r
	float R;
	// 1 - p
	float InvP;
	// 1- r
	float InvR;
};

class Dropper
{
public:
	static std::shared_ptr<Dropper> Create(float InDropChance, EDropType InDropType, DropConfig InConfig, uint16_t InSeed);

	bool Drop();

private:
	Dropper(float InDropChance, EDropType InDropType, DropConfig InConfig, uint16_t InSeed);

private:
	static std::shared_ptr<Dropper> Self;
	float							DropChance;
	EDropType						DropType;
	DropConfig						Config;
	uint16_t						Seed;

private:
	//
	typedef enum
	{
		GOOD,
		BAD
	} State;

	State CurrentState;
};