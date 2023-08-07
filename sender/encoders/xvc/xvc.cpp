#include "xvc.h"

XVCEncoder::XVCEncoder()
	: Api(xvc_encoder_api_get())
	, Params(Api->parameters_create())
{
}

XVCEncoder::~XVCEncoder()
{
}

void XVCEncoder::Init(Config& InConfig)
{
	return;
}

void XVCEncoder::Encode()
{
	return;
}