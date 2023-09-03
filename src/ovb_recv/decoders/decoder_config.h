#pragma once

#include "ovb_common/common.h"
#include "ovb_recv/decoders/avc/avc_config.h"
#include "ovb_recv/decoders/hevc/hevc_config.h"
#include "ovb_recv/decoders/ovc/ovc_config.h"
#include "ovb_recv/decoders/vvc/vvc_config.h"
#include "ovb_recv/decoders/xvc/xvc_config.h"

class DecoderConfig : public AvcDecoderConfig, public HevcDecoderConfig, public OvcDecoderConfig, public VvcDecoderConfig, public XvcDecoderConfig
{
public:
	ELogSeverity LogLevel;
};
