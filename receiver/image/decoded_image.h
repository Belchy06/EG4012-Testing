#pragma once

#include <vector>

#include "common.h"
#include "decoder_config.h"

class DecodedImage
{
public:
	std::vector<uint8_t> Bytes;
	size_t				 Size;
	DecoderConfig		 Config;
};