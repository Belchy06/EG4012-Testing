#pragma once

class IEncodeCompleteCallback
{
public:
	virtual void OnEncodeComplete(const uint8_t* Data, size_t Size) = 0;
};