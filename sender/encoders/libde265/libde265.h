#pragma once

#include "encoder.h"

class Libde265Encoder : public Encoder
{
	Libde265Encoder();
	~Libde265Encoder();

	virtual void Init(Config& InConfig) override;
	virtual void Encode() override;
};
