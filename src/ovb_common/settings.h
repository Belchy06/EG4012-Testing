#pragma once

#include "ovb_common/common.h"

class Settings
{
public:
	uint16_t	 Port;
	std::string	 File;
	ECodec		 Codec = ECodec::CODEC_UNDEFINED;
	ELogSeverity LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
};