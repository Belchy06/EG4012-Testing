#pragma once

class IEncodeCompleteCallback
{
public:
	virtual void OnEncodeComplete(uint8_t* InData, size_t InSize) = 0;
};