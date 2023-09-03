#include "encoder_factory.h"
#include "ovb_send/encoders/avc/avc_encoder.h"
#include "ovb_send/encoders/hevc/hevc_encoder.h"
#include "ovb_send/encoders/ovc/ovc_encoder.h"
#include "ovb_send/encoders/vvc/vvc_encoder.h"
#include "ovb_send/encoders/xvc/xvc_encoder.h"

std::shared_ptr<Encoder> EncoderFactory::Create(ECodec InCodec)
{
	switch (InCodec)
	{
		case CODEC_AVC:
			return std::make_shared<AvcEncoder>();
		case CODEC_HEVC:
			return std::make_shared<HevcEncoder>();
		case CODEC_OVC:
			return std::make_shared<OvcEncoder>();
		case CODEC_VVC:
			return std::make_shared<VvcEncoder>();
		case CODEC_XVC:
			return std::make_shared<XvcEncoder>();

		default:
			unimplemented();
			return nullptr;
	}
}