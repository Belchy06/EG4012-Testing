#pragma once

#include "ovb_common/common.h"
#include "ovb_recv/decoders/ovc/ovc_config.h"
#include "ovb_recv/decoders/vvc/vvc_config.h"
#include "ovb_recv/decoders/xvc/xvc_config.h"

class DecoderConfig : public OvcDecoderConfig, public VvcDecoderConfig, public XvcDecoderConfig
{
public:
	ELogSeverity LogLevel;
};
