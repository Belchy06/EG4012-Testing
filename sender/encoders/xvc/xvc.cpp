#include "xvc.h"
#include "xvc_result.h"
#include "common.h"

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

	return new XvcResult(Api->parameters_check(Params));
}

EncodeResult* XvcEncoder::Encode()
{
	return new EncodeResult();
}