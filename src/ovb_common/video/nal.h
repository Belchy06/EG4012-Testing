#pragma once

class NALU
{
public:
	NALU();
	NALU(uint8_t* InData, size_t InSize)
		: Data(InData), Size(InSize)
	{
	}

	uint8_t* Data;
	size_t	 Size;
};