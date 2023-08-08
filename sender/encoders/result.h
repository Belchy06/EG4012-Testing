#pragma once

#include "common.h"

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