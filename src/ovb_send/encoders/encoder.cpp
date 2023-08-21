#include "encoder.h"

void Encoder::RegisterEncodeCompleteCallback(IEncodeCompleteCallback* InEncoderCompleteCallback)
{
	OnEncodedImageCallback = InEncoderCompleteCallback;
}

EncodeResult* Encoder::Init(EncoderConfig& InConfig)
{
	unimplemented();
	return new EncodeResult();
}

EncodeResult* Encoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	unimplemented();
	return new EncodeResult();
}