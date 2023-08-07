#pragma once

#include "common.h"

class EncodeResult
{
public:
	virtual bool IsSuccess()
	{
		unimplemented();
		return false;
	}
};