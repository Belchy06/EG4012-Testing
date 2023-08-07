#pragma once

#include "result.h"
#include "xvc_enc_lib/xvcenc.h"

class XvcResult : public EncodeResult
{
public:
	XvcResult(xvc_enc_return_code InXvcReturn)
		: XvcReturn(InXvcReturn) {}

	virtual bool IsSuccess() { return XvcReturn != XVC_ENC_OK; }

private:
	xvc_enc_return_code XvcReturn;
};