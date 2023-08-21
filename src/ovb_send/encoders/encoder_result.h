#pragma once

#include <string>

#include "ovb_common/common.h"

class EncodeResult
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