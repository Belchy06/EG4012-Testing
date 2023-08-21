#include "ovb_send/encoders/vvc/vvc.h"

VvcEncoder::VvcEncoder()
{
}

VvcEncoder::~VvcEncoder()
{
}

EncodeResult* VvcEncoder::Init(EncoderConfig& InConfig)
{
	EncLibCommon CommonLib;

	Encoder = new EncLib(&CommonLib);

	VPS& Vps = Encoder->getVPS();
	Vps.setMaxLayers(1);
	Vps.setVPSId(0);
	Vps.setEachLayerIsAnOlsFlag(1);
	Vps.setMaxSubLayers(1);
	Vps.setAllLayersSameNumSublayersFlag(1);

	Encoder->setIntraPeriod(0);
	Encoder->setGOPSize(1);

	Encoder->create(0);

	RecBufList.push_back(new PelUnitBuf);

	Encoder->init(false, this);
}

EncodeResult* VvcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	int num_encoded;
	Encoder->encodePrep(bInLastPicture, 0, 0, IPCOLOURSPACE_UNCHANGED, RecBufList, num_encoded);

	Encoder->encode(IPCOLOURSPACE_UNCHANGED, RecBufList, num_encoded);
}