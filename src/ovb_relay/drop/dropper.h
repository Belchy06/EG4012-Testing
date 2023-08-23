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
}

class Dropper
{
public:
	static std::shared_ptr<Dropper> Create(float InDropChance, EDropType InDropType, DropConfig InConfig);

	bool Drop();

private:
	Dropper(float InDropChance, EDropType InDropType, DropConfig InConfig);

private:
	static std::shared_ptr<Dropper> Self;
	float							DropChance;
	EDropType						DropType;
	DropConfig						Config;

private:
	//
	typedef enum
	{
		GOOD,
		BAD
	} State;

	std::array<State, 2> States;
	State				 CurrentState;
};