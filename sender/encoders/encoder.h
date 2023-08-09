#pragma once

#include <vector>

#include "common.h"
#include "encoder_callback.h"
#include "encoder_config.h"
#include "encoder_result.h"

// Encoder wrapper that wraps all of the third_party codecs
class Encoder
{
public:
	void RegisterEncodeCompleteCallback(IEncodeCompleteCallback* InEncoderCompleteCallback);

	virtual EncodeResult* Init(EncoderConfig& InConfig);
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture);

protected:
	EncoderConfig			 Config;
	IEncodeCompleteCallback* OnEncodedImageCallback;
};