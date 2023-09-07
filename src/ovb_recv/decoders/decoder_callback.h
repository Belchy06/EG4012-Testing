#pragma once

#include <vector>
#include "ovb_common/image/decoded_image.h"

class IDecodeCompleteCallback
{
public:
	virtual void OnDecodeComplete(DecodedImage InImage) = 0;
};