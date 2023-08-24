#pragma once

#include <assert.h>
#include <format>
#include <iostream>
#include <string>

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

static inline std::string FormatToString(EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case CHROMA_FORMAT_MONOCHROME:
			return "CHROMA_FORMAT_MONOCHROME";
		case CHROMA_FORMAT_420:
			return "CHROMA_FORMAT_420";
		case CHROMA_FORMAT_422:
			return "CHROMA_FORMAT_422";
		case CHROMA_FORMAT_444:
			return "CHROMA_FORMAT_444";
		case CHROMA_FORMAT_UNDEFINED:
			return "CHROMA_FORMAT_UNDEFINED";
		default:
			return "UNKNOWN";
	}
}

typedef enum
{
	CODEC_XVC,
	CODEC_OVC,
	CODEC_VVC,
	CODEC_UNDEFINED = 255
} ECodec;

static inline std::string CodecToString(ECodec InCodec)
{
	switch (InCodec)
	{
		case CODEC_XVC:
			return "CODEC_XVC";
		case CODEC_OVC:
			return "CODEC_OVC";
		case CODEC_VVC:
			return "CODEC_VVC";
		case CODEC_UNDEFINED:
			return "CODEC_UNDEFINED";
		default:
			return "UNKNOWN";
	}
}

typedef enum
{
	Y,
	U,
	V
} EYuvComponent;

typedef enum
{
	LOG_SEVERITY_SILENT,
	LOG_SEVERITY_ERROR,
	LOG_SEVERITY_WARNING,
	LOG_SEVERITY_INFO,
	LOG_SEVERITY_NOTICE,
	LOG_SEVERITY_VERBOSE,
	LOG_SEVERITY_DETAILS,
} ELogSeverity;

static inline std::string SeverityToString(ELogSeverity InSeverity)
{
	switch (InSeverity)
	{
		case LOG_SEVERITY_SILENT:
			return "SILENT";
		case LOG_SEVERITY_ERROR:
			return "ERROR";
		case LOG_SEVERITY_WARNING:
			return "WARNING";
		case LOG_SEVERITY_INFO:
			return "INFO";
		case LOG_SEVERITY_NOTICE:
			return "NOTICE";
		case LOG_SEVERITY_VERBOSE:
			return "VERBOSE";
		case LOG_SEVERITY_DETAILS:
			return "DETAILS";
		default:
			return "UNKNOWN";
	}
}

namespace OvbLogging
{
	inline ELogSeverity Verbosity;
}

#define LOG(InCategory, InVerbosity, InFormat, ...)                                                                                               \
	{                                                                                                                                             \
		if (InVerbosity <= OvbLogging::Verbosity)                                                                                                 \
		{                                                                                                                                         \
			std::cout << "[ " << SeverityToString(InVerbosity) << " ] " << InCategory << ": " << std::format(InFormat, __VA_ARGS__) << std::endl; \
		}                                                                                                                                         \
	}

#define LOG_ERROR(InCategory, InVerbosity, InFormat, ...)                                                                                         \
	{                                                                                                                                             \
		if (InVerbosity <= OvbLogging::Verbosity)                                                                                                 \
		{                                                                                                                                         \
			std::cerr << "[ " << SeverityToString(InVerbosity) << " ] " << InCategory << ": " << std::format(InFormat, __VA_ARGS__) << std::endl; \
		}                                                                                                                                         \
	}
