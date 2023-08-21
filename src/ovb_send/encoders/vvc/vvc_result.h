#pragma once

#include "ovb_send/encoders/encoder_result.h"
#include "vvenc/vvenc.h"

class VvcResult : public EncodeResult
{
public:
	VvcResult(int InVvcReturn)
		: VvcReturn(InVvcReturn) {}

	virtual bool		IsSuccess() override { return VvcReturn == VVENC_OK; }
	virtual std::string Error() override
	{
		switch (VvcReturn)
		{
			case VVENC_OK:
				return "VVENC_OK";
			case VVENC_ERR_UNSPECIFIED:
				return "VVENC_ERR_UNSPECIFIED ";
			case VVENC_ERR_INITIALIZE:
				return "VVENC_ERR_INITIALIZE ";
			case VVENC_ERR_ALLOCATE:
				return "VVENC_ERR_ALLOCATE ";
			case VVENC_NOT_ENOUGH_MEM:
				return "VVENC_NOT_ENOUGH_MEM ";
			case VVENC_ERR_PARAMETER:
				return "VVENC_ERR_PARAMETER ";
			case VVENC_ERR_NOT_SUPPORTED:
				return "VVENC_ERR_NOT_SUPPORTED";
			case VVENC_ERR_RESTART_REQUIRED:
				return "VVENC_ERR_RESTART_REQUIRED";
			case VVENC_ERR_CPU:
				return "VVENC_ERR_CPU ";
			default:
				return "";
		}
	}

private:
	int VvcReturn;
};