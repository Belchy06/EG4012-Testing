#pragma once

#include <vector>

#include "ovb_common/common.h"
#include "ovb_recv/decoders/decoder_config.h"

class DecodedImage
{
public:
	std::vector<uint8_t> Bytes;
	size_t				 Size;
	DecoderConfig		 Config;
};