#include "ovc.h"
#include "encoder_factory.h"
#include "libde265.h"
#include "xvc.h"

std::shared_ptr<Encoder> EncoderFactory::Create(ECodec InCodec)
{
	if (InCodec == ECodec::CODEC_H265)
	{
		return std::make_shared<Libde265Encoder>();
	}
	else if (InCodec == ECodec::CODEC_XVC)
	{
		return std::make_shared<XvcEncoder>();
	}
	else if (InCodec == ECodec::CODEC_OVC)
	{
		return std::make_shared<OvcEncoder>();
	}
	return nullptr;
}