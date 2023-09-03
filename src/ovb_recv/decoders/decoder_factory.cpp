#include "ovb_recv/decoders/avc/avc_decoder.h"
#include "ovb_recv/decoders/hevc/hevc_decoder.h"
#include "ovb_recv/decoders/ovc/ovc_decoder.h"
#include "decoder_factory.h"
#include "ovb_recv/decoders/vvc/vvc_decoder.h"
#include "ovb_recv/decoders/xvc/xvc_decoder.h"

std::shared_ptr<Decoder> DecoderFactory::Create(ECodec InCodec)
{
	switch (InCodec)
	{
		case CODEC_AVC:
			return std::make_shared<AvcDecoder>();
		case CODEC_HEVC:
			return std::make_shared<HevcDecoder>();
		case CODEC_OVC:
			return std::make_shared<OvcDecoder>();
		case CODEC_VVC:
			return std::make_shared<VvcDecoder>();
		case CODEC_XVC:
			return std::make_shared<XvcDecoder>();
		default:
			unimplemented();
			return nullptr;
	}
}