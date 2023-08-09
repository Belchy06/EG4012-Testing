#pragma once

class IEncodeCompleteCallback
{
public:
	virtual void OnEncodeComplete(const uint8_t* InData, size_t InSize) = 0;
};