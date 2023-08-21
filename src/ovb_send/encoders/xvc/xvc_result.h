#pragma once

#include "ovb_send/encoders/encoder_result.h"
#include "xvc_enc_lib/xvcenc.h"

class XvcResult : public EncodeResult
{
public:
	XvcResult(xvc_enc_return_code InXvcReturn)
		: XvcReturn(InXvcReturn) {}

	virtual bool		IsSuccess() override { return XvcReturn == XVC_ENC_OK; }
	virtual std::string Error() override
	{
		switch (XvcReturn)
		{
			case XVC_ENC_OK:
				return "XVC_ENC_OK";
			case XVC_ENC_NO_MORE_OUTPUT:
				return "XVC_ENC_NO_MORE_OUTPUT";
			case XVC_ENC_INVALID_ARGUMENT:
				return "XVC_ENC_INVALID_ARGUMENT";
			case XVC_ENC_INVALID_PARAMETER:
				return "XVC_ENC_INVALID_PARAMETER";
			case XVC_ENC_SIZE_TOO_SMALL:
				return "XVC_ENC_SIZE_TOO_SMALL";
			case XVC_ENC_UNSUPPORTED_CHROMA_FORMAT:
				return "XVC_ENC_UNSUPPORTED_CHROMA_FORMAT";
			case XVC_ENC_BITDEPTH_OUT_OF_RANGE:
				return "XVC_ENC_BITDEPTH_OUT_OF_RANGE";
			case XVC_ENC_COMPILED_BITDEPTH_TOO_LOW:
				return "XVC_ENC_COMPILED_BITDEPTH_TOO_LOW";
			case XVC_ENC_FRAMERATE_OUT_OF_RANGE:
				return "XVC_ENC_FRAMERATE_OUT_OF_RANGE";
			case XVC_ENC_QP_OUT_OF_RANGE:
				return "XVC_ENC_QP_OUT_OF_RANGE";
			case XVC_ENC_SUB_GOP_LENGTH_TOO_LARGE:
				return "XVC_ENC_SUB_GOP_LENGTH_TOO_LARGE";
			case XVC_ENC_DEBLOCKING_SETTINGS_INVALID:
				return "XVC_ENC_DEBLOCKING_SETTINGS_INVALID";
			case XVC_ENC_TOO_MANY_REF_PICS:
				return "XVC_ENC_TOO_MANY_REF_PICS";
			case XVC_ENC_SIZE_TOO_LARGE:
				return "XVC_ENC_SIZE_TOO_LARGE";
			case XVC_ENC_NO_SUCH_PRESET:
				return "XVC_ENC_NO_SUCH_PRESET";
			default:
				return "";
		}
	}

private:
	xvc_enc_return_code XvcReturn;
};