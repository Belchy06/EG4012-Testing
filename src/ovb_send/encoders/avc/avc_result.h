#pragma once

#include "ovb_send/encoders/encoder_result.h"
#include "wels/codec_def.h"

class AvcResult : public EncodeResult
{
public:
	AvcResult(CM_RETURN InAvcReturn)
		: AvcReturn(InAvcReturn) {}

	virtual bool		IsSuccess() override { return AvcReturn == cmResultSuccess; }
	virtual std::string Error() override
	{
		switch (AvcReturn)
		{
			case cmResultSuccess:
				return "cmResultSuccess";
			case cmInitParaError:
				return "cmInitParaError";
			case cmUnknownReason:
				return "cmUnknownReason";
			case cmMallocMemeError:
				return "cmMallocMemeError";
			case cmInitExpected:
				return "cmInitExpected";
			case cmUnsupportedData:
				return "cmUnsupportedData";
			default:
				return "";
		}
	}

private:
	CM_RETURN AvcReturn;
};