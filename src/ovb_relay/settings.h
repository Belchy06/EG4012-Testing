#pragma once

#include <string>

#include "ovb_common/common.h"

typedef enum
{
	LOSS_CONTINUOUS,
	LOSS_SIMPLE_BURSTY,
	LOSS_COMPLEX_BURSTY,
} EDropType;

static inline std::string DropTypeToString(EDropType InDropType)
{
	switch (InDropType)
	{
		case LOSS_CONTINUOUS:
			return "LOSS_CONTINUOUS";
		case LOSS_SIMPLE_BURSTY:
			return "LOSS_SIMPLE_BURSTY";
		case LOSS_COMPLEX_BURSTY:
			return "LOSS_COMPLEX_BURSTY";
		default:
			return "UNKNOWN";
	}
}

class BurstyDropSettings
{
public:
	// G1, G2, G3, G4
	float DropChanceGood;
	// b
	float DropChanceBad;
	// q
	float Q;
};

class SimpleBurstyDropSettings
{
public:
	// p
	float P;
};

class ComplexBurstyDropSettings
{
public:
	// p1
	float P1;
	// p2
	float P2;
	// p3
	float P3;
	// p4
	float P4;
};

class ContinuousDropSettings
{
public:
	float DropChance;
};

class DropSettings : public ContinuousDropSettings, public BurstyDropSettings, public SimpleBurstyDropSettings, public ComplexBurstyDropSettings
{
public:
	// Random Seed
	uint16_t Seed;
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