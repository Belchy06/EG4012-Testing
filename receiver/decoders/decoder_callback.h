#pragma once

#include <vector>
#include "decoded_image.h"

class IDecodeCompleteCallback
{
public:
	virtual void OnDecodeComplete(DecodedImage InImage) = 0;
};