#pragma once

#include "ovb_common/common.h"

class Settings
{
public:
	std::string	 File;
	ECodec		 Codec = ECodec::CODEC_UNDEFINED;
	ELogSeverity LogLevel = ELogSeverity::SEVERITY_LOG;
};