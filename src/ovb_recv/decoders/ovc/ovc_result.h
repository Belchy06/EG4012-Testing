#pragma once

#include "ovb_recv/decoders/decoder_result.h"
#include "xvc_dec_lib/xvcdec.h"

class OvcResult : public DecodeResult
{
public:
	OvcResult(ovc_dec_result InOvcReturn)
		: OvcReturn(InOvcReturn) {}

	virtual bool		IsSuccess() override { return OvcReturn == OVC_DEC_OK; }
	virtual std::string Error() override
	{
		switch (OvcReturn)
		{
			case OVC_DEC_OK:
				return "OVC_DEC_OK";
			default:
				return "";
		}
	}

private:
	ovc_dec_result OvcReturn;
};