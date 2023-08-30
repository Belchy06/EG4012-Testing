#include "encoder.h"

void Encoder::RegisterEncodeCompleteCallback(IEncodeCompleteCallback* InEncoderCompleteCallback)
{
	OnEncodedImageCallback = InEncoderCompleteCallback;
}

EncodeResult* Encoder::Init(EncoderConfig& InConfig)
{
	unimplemented();
	return new EncodeResult();
}

EncodeResult* Encoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	unimplemented();
	return new EncodeResult();
}

int Encoder::ScaleX(int InX, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_400:
			return 0;
		case EChromaFormat::CHROMA_FORMAT_444:
			return InX;
		case EChromaFormat::CHROMA_FORMAT_420:
		case EChromaFormat::CHROMA_FORMAT_422:
			return InX >> 1;
		default:
			return 0;
	}
}

int Encoder::ScaleY(int InY, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_400:
			return 0;
		case EChromaFormat::CHROMA_FORMAT_444:
		case EChromaFormat::CHROMA_FORMAT_422:
			return InY;
		case EChromaFormat::CHROMA_FORMAT_420:
			return InY >> 1;
		default:
			return 0;
	}
}