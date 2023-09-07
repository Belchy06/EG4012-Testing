#pragma once

#include <string>

class HevcEncoderConfig
{
public:
	std::string HevcSopStructure = "intra";
	int			HevcQP = 27;
};