#pragma once

#include "result.h"
#include "libde265/de265.h"

class Libde265Result : public EncodeResult
{
public:
	Libde265Result(de265_error InLibde265Return)
		: Libde265Return(InLibde265Return) {}

	virtual bool IsSuccess() { return Libde265Return == DE265_OK; }

private:
	de265_error Libde265Return;
};