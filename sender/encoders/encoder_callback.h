#pragma once

class IEncodeCompleteCallback
{
public:
	virtual void OnEncodeComplete(uint8_t* Data, size_t Size) = 0;
};