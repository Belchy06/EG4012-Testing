#include "encoder_factory.h"
#include "ovb_send/encoders/avc/avc.h"
#include "ovb_send/encoders/ovc/ovc.h"
#include "ovb_send/encoders/vvc/vvc.h"
#include "ovb_send/encoders/xvc/xvc.h"

std::shared_ptr<Encoder> EncoderFactory::Create(ECodec InCodec)
{
	if (InCodec == ECodec::CODEC_VVC)
	{
		return std::make_shared<VvcEncoder>();
	}
	else if (InCodec == ECodec::CODEC_XVC)
	{
		return std::make_shared<XvcEncoder>();
	}
	else if (InCodec == ECodec::CODEC_OVC)
	{
		return std::make_shared<OvcEncoder>();
	}
	else if (InCodec == ECodec::CODEC_AVC)
	{
		return std::make_shared<AvcEncoder>();
	}
	return nullptr;
}