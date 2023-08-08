#include "xvc.h"
#include "xvc_result.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

XvcEncoder::XvcEncoder()
	: Api(xvc_encoder_api_get())
	, Params(Api->parameters_create())
	, Encoder(nullptr)
{
}

XvcEncoder::~XvcEncoder()
{
	if (Encoder)
	{
		Api->encoder_destroy(Encoder);
		Encoder = nullptr;
	}

	if (Params)
	{
		Api->parameters_destroy(Params);
		Params = nullptr;
	}
}

EncodeResult* XvcEncoder::Init(EncoderConfig& InConfig)
{
	Api->parameters_set_default(Params);

	if (InConfig.Width)
	{
		Params->width = InConfig.Width;
	}

	if (InConfig.Height)
	{
		Params->height = InConfig.Height;
	}

	if (InConfig.Framerate)
	{
		Params->framerate = InConfig.Framerate;
	}

	if (InConfig.BitDepth)
	{
		Params->input_bitdepth = InConfig.BitDepth;
	}

	if (InConfig.Format != EChromaFormat::CHROMA_FORMAT_UNDEFINED)
	{
		// clang-format off
        if(InConfig.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME) {
            Params->chroma_format = xvc_enc_chroma_format::XVC_ENC_CHROMA_FORMAT_MONOCHROME;
        } else if(InConfig.Format == EChromaFormat::CHROMA_FORMAT_420) {
            Params->chroma_format = xvc_enc_chroma_format::XVC_ENC_CHROMA_FORMAT_420;
        } else if(InConfig.Format == EChromaFormat::CHROMA_FORMAT_422) {
            Params->chroma_format = xvc_enc_chroma_format::XVC_ENC_CHROMA_FORMAT_422;
        } else if(InConfig.Format == EChromaFormat::CHROMA_FORMAT_444) {
            Params->chroma_format = xvc_enc_chroma_format::XVC_ENC_CHROMA_FORMAT_444;
        }
		// clang-format on
	}

	xvc_enc_return_code Result = Api->parameters_check(Params);
	if (Result != XVC_ENC_OK)
	{
		return new XvcResult(Result);
	}

	Encoder = Api->encoder_create(Params);
	if (!Encoder)
	{
		std::cerr << "Error: Failed to allocate encoder" << std::endl;
		std::exit(-1);
	}

	return new XvcResult(Result);
}

EncodeResult* XvcEncoder::Encode(std::istream* InStream)
{
	return new EncodeResult();
}