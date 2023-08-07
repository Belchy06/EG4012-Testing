#pragma once

#include "common.h"

// Encoder wrapper that wraps all of the third_party codecs
class Encoder
{
public:
	virtual void Encode() { unimplemented(); }
};