#pragma once

#include <string>

#include "vvdec/vvdec.h"

class VvcDecoderConfig
{
public:
	vvdecErrHandlingFlags VvcErrorHandlingFlags;
};

inline std::string ErrorHandlingFlagsToString(vvdecErrHandlingFlags InErrorHandlingFlags)
{
	switch (InErrorHandlingFlags)
	{
		case VVDEC_ERR_HANDLING_OFF:
			return "VVDEC_ERR_HANDLING_OFF";
		case VVDEC_ERR_HANDLING_TRY_CONTINUE:
			return "VVDEC_ERR_HANDLING_TRY_CONTINUE";
		default:
			return "";
	}
}