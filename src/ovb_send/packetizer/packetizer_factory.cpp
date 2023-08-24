#include "ovb_common/common.h"

#include "ovb_send/packetizer/packetizer_factory.h"
#include "ovb_send/packetizer/xvc/xvc_packetizer.h"
#include "ovb_send/packetizer/vvc/vvc_packetizer.h"
#include "ovb_send/packetizer/ovc/ovc_packetizer.h"

std::shared_ptr<Packetizer> PacketizerFactory::Create(ECodec InCodec)
{
	switch (InCodec)
	{
		case CODEC_XVC:
			return std::make_shared<XvcPacketizer>();
		case CODEC_VVC:
			return std::make_shared<VvcPacketizer>();
		case CODEC_OVC:
			return std::make_shared<OvcPacketizer>();
		default:
			unimplemented();
			return nullptr;
	}
}