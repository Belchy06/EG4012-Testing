#pragma once

class AvcEncoderConfig
{
public:
	int			 AvcBitrate = -1;
	int			 AvcQP = -1;
	unsigned int AvcIntraPeriod = 1;
	int			 AvcNumRefFrame = 0;
};