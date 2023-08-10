#pragma once

#include "encoder_result.h"
#include "bvc_enc/result.h"

class BvcResult : public EncodeResult
{
public:
	BvcResult(bvc_enc_result InBvcReturn)
		: BvcReturn(InBvcReturn) {}

	virtual bool		IsSuccess() override { return BvcReturn == BVC_ENC_OK; }
	virtual std::string Error() override
	{
		switch (BvcReturn)
		{
			case BVC_ENC_OK:
				return "BVC_ENC_OK";
			case BVC_ENC_INVALID_DIMENSIONS:
				return "BVC_ENC_INVALID_DIMENSIONS";
			case BVC_ENC_INVALID_FORMAT:
				return "BVC_ENC_INVALID_FORMAT";
			default:
				return "";
		}
	}

private:
	bvc_enc_result BvcReturn;
};