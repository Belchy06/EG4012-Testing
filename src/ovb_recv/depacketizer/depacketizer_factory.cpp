#include "ovb_common/common.h"

#include "ovb_recv/depacketizer/depacketizer_factory.h"
#include "ovb_recv/depacketizer/avc/avc_depacketizer.h"
#include "ovb_recv/depacketizer/ovc/ovc_depacketizer.h"
#include "ovb_recv/depacketizer/vvc/vvc_depacketizer.h"
#include "ovb_recv/depacketizer/xvc/xvc_depacketizer.h"

std::shared_ptr<Depacketizer> DepacketizerFactory::Create(ECodec InCodec)
{
	switch (InCodec)
	{
		case CODEC_AVC:
			return std::make_shared<AvcDepacketizer>();
		case CODEC_OVC:
			return std::make_shared<OvcDepacketizer>();
		case CODEC_VVC:
			return std::make_shared<VvcDepacketizer>();
		case CODEC_XVC:
			return std::make_shared<XvcDepacketizer>();
		default:
			unimplemented();
			return nullptr;
	}
}