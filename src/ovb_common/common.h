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
	CODEC_XVC,
	CODEC_OVC,
	CODEC_VVC,
	CODEC_UNDEFINED = 255
} ECodec;

typedef enum
{
	Y,
	U,
	V
} EYuvComponent;

typedef enum
{
	SEVERITY_NONE,
	SEVERITY_LOG,
	SEVERITY_VERBOSE,
	SEVERITY_VERY_VERBOSE
} ELogSeverity;
