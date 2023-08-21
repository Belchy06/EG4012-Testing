#pragma once

#include <vector>

#include "ovb_send/encoders/encoder.h"
#include "EncoderLib/EncLibCommon.h"
#include "EncoderLib/EncLib.h"

class VvcEncoder : public Encoder, public AUWriterIf
{
public:
	VvcEncoder();
	~VvcEncoder();

	virtual EncodeResult* Init(EncoderConfig& InConfig) override;
	virtual EncodeResult* Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture) override;

private:
	// AUWriteIf interface
	virtual void outputAU(const AccessUnit& au) override;

private:
	EncLib* Encoder;

	std::list<PelUnitBuf*> RecBufList;
	PelStorage*			   OrgPic;
};