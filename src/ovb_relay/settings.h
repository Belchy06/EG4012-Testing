#pragma once

#include <string>

#include "ovb_common/common.h"

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

	// Drop
	float DropChance;

	// Tamper
	float TamperChance;
};