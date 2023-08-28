#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "ovb_common/common.h"
#include "avc.h"
#include "avc_result.h"

#define LogAvcEncoder "LogAvcEncoder"

AvcEncoder::AvcEncoder()
	: Params(new SEncParamBase()), Encoder(nullptr)
{
}

AvcEncoder::~AvcEncoder()
{
	if (Encoder)
	{
		Encoder = nullptr;
	}

	if (Params)
	{
		Params = nullptr;
	}
}

EncodeResult* AvcEncoder::Init(EncoderConfig& InConfig)
{
	return new AvcResult(cmResultSuccess);
}

EncodeResult* AvcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	return new AvcResult(cmResultSuccess);
}

#undef LogAvcEncoder