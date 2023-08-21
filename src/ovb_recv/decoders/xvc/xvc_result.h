#pragma once

#include "ovb_recv/decoders/decoder_result.h"
#include "xvc_dec_lib/xvcdec.h"

class XvcResult : public DecodeResult
{
public:
	XvcResult(xvc_dec_return_code InXvcReturn)
		: XvcReturn(InXvcReturn) {}

	virtual bool		IsSuccess() override { return XvcReturn == XVC_DEC_OK; }
	virtual std::string Error() override
	{
		switch (XvcReturn)
		{
			case XVC_DEC_OK:
				return "XVC_DEC_OK";
			case XVC_DEC_NO_DECODED_PIC:
				return "XVC_DEC_NO_DECODED_PIC";
			case XVC_DEC_NOT_CONFORMING:
				return "XVC_DEC_NOT_CONFORMING";
			case XVC_DEC_INVALID_ARGUMENT:
				return "XVC_DEC_INVALID_ARGUMENT";
			case XVC_DEC_INVALID_PARAMETER:
				return "XVC_DEC_INVALID_PARAMETER";
			case XVC_DEC_FRAMERATE_OUT_OF_RANGE:
				return "XVC_DEC_FRAMERATE_OUT_OF_RANGE";
			case XVC_DEC_BITDEPTH_OUT_OF_RANGE:
				return "XVC_DEC_BITDEPTH_OUT_OF_RANGE";
			case XVC_DEC_BITSTREAM_VERSION_HIGHER_THAN_DECODER:
				return "XVC_DEC_BITSTREAM_VERSION_HIGHER_THAN_DECODER";
			case XVC_DEC_NO_SEGMENT_HEADER_DECODED:
				return "XVC_DEC_NO_SEGMENT_HEADER_DECODED";
			case XVC_DEC_BITSTREAM_BITDEPTH_TOO_HIGH:
				return "XVC_DEC_BITSTREAM_BITDEPTH_TOO_HIGH";
			case XVC_DEC_BITSTREAM_VERSION_LOWER_THAN_SUPPORTED_BY_DECODER:
				return "XVC_DEC_BITSTREAM_VERSION_LOWER_THAN_SUPPORTED_BY_DECODER";
			default:
				return "";
		}
	}

private:
	xvc_dec_return_code XvcReturn;
};