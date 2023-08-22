#pragma once

#include "ovb_recv/decoders/decoder_result.h"
#include "vvdec/vvdec.h"

class VvcResult : public DecodeResult
{
public:
	VvcResult(int InVvcReturn)
		: VvcReturn(InVvcReturn) {}

	virtual bool		IsSuccess() override { return VvcReturn == VVDEC_OK; }
	virtual std::string Error() override
	{
		switch (VvcReturn)
		{
			case VVDEC_OK:
				return "VVDEC_OK";
			case VVDEC_ERR_UNSPECIFIED:
				return "VVDEC_ERR_UNSPECIFIED";
			case VVDEC_ERR_INITIALIZE:
				return "VVDEC_ERR_INITIALIZE";
			case VVDEC_ERR_ALLOCATE:
				return "VVDEC_ERR_ALLOCATE";
			case VVDEC_ERR_DEC_INPUT:
				return "VVDEC_ERR_DEC_INPUT";
			case VVDEC_NOT_ENOUGH_MEM:
				return "VVDEC_NOT_ENOUGH_MEM";
			case VVDEC_ERR_PARAMETER:
				return "VVDEC_ERR_PARAMETER";
			case VVDEC_ERR_NOT_SUPPORTED:
				return "VVDEC_ERR_NOT_SUPPORTED";
			case VVDEC_ERR_RESTART_REQUIRED:
				return "VVDEC_ERR_RESTART_REQUIRED";
			case VVDEC_ERR_CPU:
				return "VVDEC_ERR_CPU";
			case VVDEC_TRY_AGAIN:
				return "VVDEC_TRY_AGAIN";
			case VVDEC_EOF:
				return "VVDEC_EOF";
			default:
				return "";
		}
	}

private:
	int VvcReturn;
};