#pragma once

#include <iostream>

#include "ovb_recv/image/decoded_image.h"

class Y4mWriter
{
public:
	Y4mWriter(std::ostream* InStream);
	void WriteImageHeader(DecodedImage& InImage);
	void WriteImage(DecodedImage& InImage);

private:
	void WriteFileHeader(DecodedImage& InImage);

private:
	std::ostream* Stream;
	bool		  bWriteFileHeader = true;
};