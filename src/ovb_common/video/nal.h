#pragma once

class NALU
{
public:
	NALU();
	NALU(uint8_t* InData, size_t InSize)
		: Data(InData), Size(InSize)
	{
	}

	NALU(const uint8_t* InData, size_t InSize)
		: Size(InSize)
	{
		Data = new uint8_t[InSize]{ 0 };
		memcpy(Data, InData, InSize);
	}

	uint8_t* Data;
	size_t	 Size;
};