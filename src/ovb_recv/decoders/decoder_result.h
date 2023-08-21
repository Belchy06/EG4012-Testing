#pragma once

#include "ovb_common/common.h"

class DecodeResult
{
public:
	virtual bool IsSuccess()
	{
		return false;
	}

	virtual std::string Error()
	{
		return "";
	}
};