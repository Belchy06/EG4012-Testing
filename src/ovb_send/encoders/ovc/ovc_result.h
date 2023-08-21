#pragma once

#include "ovb_send/encoders/encoder_result.h"
#include "ovc_enc/result.h"

class OvcResult : public EncodeResult
{
public:
	OvcResult(ovc_enc_result InOvcReturn)
		: OvcReturn(InOvcReturn) {}

	virtual bool		IsSuccess() override { return OvcReturn == OVC_ENC_OK; }
	virtual std::string Error() override
	{
		switch (OvcReturn)
		{
			case OVC_ENC_OK:
				return "OVC_ENC_OK";
			case OVC_ENC_INVALID_DIMENSIONS:
				return "OVC_ENC_INVALID_DIMENSIONS";
			case OVC_ENC_INVALID_FORMAT:
				return "OVC_ENC_INVALID_FORMAT";
			default:
				return "";
		}
	}

private:
	ovc_enc_result OvcReturn;
};