#pragma once

#include <string>

#include "ovb_common/common.h"

typedef enum
{
	DSIS,
} EMethod;

static inline std::string MethodToString(EMethod InMethod)
{
	switch (InMethod)
	{
		case DSIS:
			return "DOUBLE_STIMULUS_IMPAIRMENT_SCALE";
		default:
			return "UNKNOWN";
	}
}

class EditorSettings
{
public:
	std::string OriginalFile;

	std::string DistortedFile;

	std::string OutputFile;

	EMethod Method;

	// Logging severity
	ELogSeverity LogLevel;

	float Framerate;
};