#pragma once

#include "decoder_result.h"
#include "xvc_dec_lib/xvcdec.h"

class BvcResult : public DecodeResult
{
public:
	BvcResult(bvc_dec_result InBvcReturn)
		: BvcReturn(InBvcReturn) {}

	virtual bool		IsSuccess() override { return BvcReturn == BVC_DEC_OK; }
	virtual std::string Error() override
	{
		switch (BvcReturn)
		{
			case BVC_DEC_OK:
				return "BVC_DEC_OK";
			default:
				return "";
		}
	}

private:
	bvc_dec_result BvcReturn;
};