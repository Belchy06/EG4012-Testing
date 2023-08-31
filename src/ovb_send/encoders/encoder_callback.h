#pragma once

#include <vector>

#include "ovb_common/video/nal.h"

class IEncodeCompleteCallback
{
public:
	virtual void OnEncodeComplete(std::vector<NALU> InNALUs) = 0;
};