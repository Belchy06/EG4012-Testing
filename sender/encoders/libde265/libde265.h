#pragma once

#include "encoder.h"

class Libde265Encoder : public Encoder
{
	virtual void Encode() override;
};
