#include "common.h"
#include "y4m_writer.h"

Y4mWriter::Y4mWriter(std::ostream* InStream)
	: Stream(InStream)
{
}

void Y4mWriter::WriteImageHeader(DecodedImage& InImage)
{
	if (bWriteFileHeader)
	{
		WriteFileHeader(InImage);
		bWriteFileHeader = false;
	}
	(*Stream) << "FRAME\n";
}

void Y4mWriter::WriteImage(DecodedImage& InImage)
{
	Stream->write((char*)InImage.Bytes.data(), InImage.Size);
	Stream->flush();
}

void Y4mWriter::WriteFileHeader(DecodedImage& InImage)
{
	std::string ChromaStr;
	if (InImage.Config.Format == EChromaFormat::CHROMA_FORMAT_420)
	{
		ChromaStr = "420";
	}
	else if (InImage.Config.Format == EChromaFormat::CHROMA_FORMAT_422)
	{
		ChromaStr = "422";
	}
	else if (InImage.Config.Format == EChromaFormat::CHROMA_FORMAT_420)
	{
		ChromaStr = "444";
	}
	else if (InImage.Config.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME)
	{
		ChromaStr = "mono";
	}
	else
	{
		assert(0);
	}

	(*Stream)
		<< "YUV4MPEG2 "
		<< "W" << InImage.Config.Width << " H" << InImage.Config.Height << " "
		<< "F" << InImage.Config.FramerateNum << ":" << InImage.Config.FramerateDenom << " "
		<< "Ip "
		<< "C" << ChromaStr;

	if (InImage.Config.BitDepth > 8)
	{
		(*Stream) << "p" << InImage.Config.BitDepth;
	}

	(*Stream) << "\n";
}