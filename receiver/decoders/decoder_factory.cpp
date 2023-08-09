#include "decoder_factory.h"
#include "libde265.h"
#include "xvc.h"

std::shared_ptr<Decoder> DecoderFactory::Create(ECodec InCodec)
{
	if (InCodec == ECodec::CODEC_H265)
	{
		// return std::make_shared<Libde265Decoder>();
		return nullptr;
	}
	else if (InCodec == ECodec::CODEC_XVC)
	{
		return std::make_shared<XvcDecoder>();
	}
	return nullptr;
}