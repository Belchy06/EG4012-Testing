#pragma once

#include <string>

#include "ovb_common/common.h"

typedef enum
{
	LOSS_CONTINUOUS,
	LOSS_BURSTY,
} EDropType;

static inline std::string DropTypeToString(EDropType InDropType)
{
	switch (InDropType)
	{
		case LOSS_CONTINUOUS:
			return "LOSS_CONTINUOUS";
		case LOSS_BURSTY:
			return "LOSS_BURSTY";
		default:
			return "UNKNOWN";
	}
}

class DropSettings
{
public:
	// h_g
	float DropChanceGood;
	// h_b
	float DropChanceBad;
	// p
	float P;
	// r
	float R;

	float DropChance;

	// Random Seed
	uint16_t Seed;

	bool IsSet = false;
};

class TamperSettings
{
public:
	float TamperChance;

	// Random Seed
	uint16_t Seed;
};

class RelaySettings
{
public:
	// The IP to forward incoming packets to
	std::string SendIP;
	// The port to forward incoming packets to
	uint16_t SendPort;

	// The port to listen on for incoming packets
	uint16_t RecvPort;

	// Logging severity
	ELogSeverity LogLevel;

	// Drop Model
	EDropType DropType;

	// Drop
	DropSettings DropOptions;

	// Tamper
	TamperSettings TamperOptions;
};