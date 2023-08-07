#include "encoder_factory.h"

#include "xvc.h"
#include "libde265.h"

Encoder* EncoderFactory::Create(ECodec InCodec)
{
	if (InCodec == ECodec::CODEC_H265)
	{
		return new Libde265Encoder();
	}
	else if (InCodec == ECodec::CODEC_XVC)
	{
		return new XVCEncoder();
	}
}