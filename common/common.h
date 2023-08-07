#pragma once

#include <assert.h>

#ifndef unimplemented
	#define unimplemented() assert(!true)
#endif

typedef enum
{
	CHROMA_FORMAT_MONOCHROME,
	CHROMA_FORMAT_420,
	CHROMA_FORMAT_422,
	CHROMA_FORMAT_444,
	CHROMA_FORMAT_UNDEFINED = 255
} EChromaFormat;

typedef enum
{
	CODEC_H265,
	CODEC_AV1,
	CODEC_XVC,
	CODEC_UNDEFINED = 255
} ECodec;
