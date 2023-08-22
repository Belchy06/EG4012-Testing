#include "ovb_recv/decoders/ovc/ovc.h"
#include "decoder_factory.h"
#include "ovb_recv/decoders/vvc/vvc.h"
#include "ovb_recv/decoders/xvc/xvc.h"

std::shared_ptr<Decoder> DecoderFactory::Create(ECodec InCodec)
{
	if (InCodec == ECodec::CODEC_VVC)
	{
		return std::make_shared<VvcDecoder>();
	}
	else if (InCodec == ECodec::CODEC_XVC)
	{
		return std::make_shared<XvcDecoder>();
	}
	else if (InCodec == ECodec::CODEC_OVC)
	{
		return std::make_shared<OvcDecoder>();
	}
	return nullptr;
}