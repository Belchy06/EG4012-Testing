#include "ovb_send/encoders/vvc/vvc.h"

VvcEncoder::VvcEncoder()
	: Params(new vvenc_config())
	, Encoder(nullptr)
	, SequenceNumber(0)
{
}

VvcEncoder::~VvcEncoder()
{
	if (Encoder)
	{
		Encoder = nullptr;
	}

	if (Params)
	{
		Params = nullptr;
	}
}

EncodeResult* VvcEncoder::Init(EncoderConfig& InConfig)
{
	Config = InConfig;

	vvenc_init_default(Params, InConfig.Width, InConfig.Height, (int)InConfig.Framerate, VVENC_RC_OFF, VVENC_AUTO_QP, VVENC_MEDIUM);

	// TODO (belchy06): Support > 8bit profiles
	Params->m_inputBitDepth[0] = 8;
	Params->m_inputBitDepth[1] = 8;

	vvencChromaFormat internalFormat = VVENC_NUM_CHROMA_FORMAT;
	switch (InConfig.Format)
	{
		case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
			internalFormat = VVENC_CHROMA_400;
			break;
		case EChromaFormat::CHROMA_FORMAT_420:
			internalFormat = VVENC_CHROMA_420;
			break;
		case EChromaFormat::CHROMA_FORMAT_422:
			internalFormat = VVENC_CHROMA_422;
			break;
		case EChromaFormat::CHROMA_FORMAT_444:
			internalFormat = VVENC_CHROMA_444;
			break;
	}
	Params->m_internChromaFormat = internalFormat;

	Encoder = vvenc_encoder_create();

	int Result = vvenc_encoder_open(Encoder, Params);
	if (Result != VVENC_OK)
	{
		vvenc_encoder_close(Encoder);
		return new VvcResult(Result);
	}

	vvenc_get_config(Encoder, Params);

	return new VvcResult(VVENC_OK);
}

EncodeResult* VvcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	// --- allocate memory for output packets
	vvencAccessUnit AU;
	vvenc_accessUnit_default(&AU);
	const int auSizeScale = Params->m_internChromaFormat <= VVENC_CHROMA_420 ? 2 : 3;
	vvenc_accessUnit_alloc_payload(&AU, auSizeScale * Params->m_SourceWidth * Params->m_SourceHeight + 1024);

	// --- allocate memory for YUV input picture
	vvencYUVBuffer YUVInputBuffer;
	vvenc_YUVBuffer_default(&YUVInputBuffer);
	vvenc_YUVBuffer_alloc_buffer(&YUVInputBuffer, Params->m_internChromaFormat, Params->m_SourceWidth, Params->m_SourceHeight);

	const int Start = Params->m_RCPass > 0 ? Params->m_RCPass - 1 : 0;
	const int End = Params->m_RCPass > 0 ? Params->m_RCPass : Params->m_RCNumPasses;
	for (int Pass = Start; Pass < End; Pass++)
	{
		int Result = vvenc_init_pass(Encoder, Pass, "");
		if (Result != VVENC_OK)
		{
			vvenc_YUVBuffer_free_buffer(&YUVInputBuffer);
			vvenc_accessUnit_free_payload(&AU);
			vvenc_encoder_close(Encoder);
			return new VvcResult(Result);
		}

		YUVInputBuffer.sequenceNumber = SequenceNumber;
		YUVInputBuffer.cts = (Params->m_TicksPerSecond > 0);
		YUVInputBuffer.ctsValid = true;

		uint8_t*			 SrcBytes = InPictureBytes.data();
		size_t				 FrameSize = Config.Width * Config.Height;
		std::vector<int16_t> Y;
		for (size_t i = 0; i < FrameSize; i++)
		{
			Y.push_back((int16_t)SrcBytes[i]);
		}

		std::vector<int16_t> U;
		switch (Config.Format)
		{
			case EChromaFormat::CHROMA_FORMAT_420:
				for (size_t i = 0; i < FrameSize / 4; i++)
				{
					// (int16_t)SrcBytes[i + FrameSize] > 256 || (int16_t)SrcBytes[i + FrameSize] < 0
					U.push_back((int16_t)SrcBytes[i + FrameSize]);
				}
				break;
			case EChromaFormat::CHROMA_FORMAT_422:
				for (size_t i = 0; i < (size_t)((Config.Width / 2) * Config.Height); i++)
				{
					U.push_back((int16_t)SrcBytes[i + FrameSize]);
				}
				break;
			case EChromaFormat::CHROMA_FORMAT_444:
				for (size_t i = 0; i < FrameSize; i++)
				{
					U.push_back((int16_t)SrcBytes[i + FrameSize]);
				}
				break;
			case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
			case EChromaFormat::CHROMA_FORMAT_UNDEFINED:
			default:
				break;
		}

		std::vector<int16_t> V;
		switch (Config.Format)
		{
			case EChromaFormat::CHROMA_FORMAT_420:
				for (size_t i = 0; i < FrameSize / 4; i++)
				{
					V.push_back((int16_t)SrcBytes[i + (5 / 4) * FrameSize]);
				}
				break;
			case EChromaFormat::CHROMA_FORMAT_422:
				for (size_t i = 0; i < (size_t)((Config.Width / 2) * Config.Height); i++)
				{
					V.push_back((int16_t)SrcBytes[i + (Config.Width * Config.Width) + (3 * Config.Width * Config.Height) + (2 * Config.Height * Config.Height)]);
				}
				break;
			case EChromaFormat::CHROMA_FORMAT_444:
				for (size_t i = 0; i < FrameSize; i++)
				{
					V.push_back((int16_t)SrcBytes[i + 2 * FrameSize]);
				}
				break;
			case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
			case EChromaFormat::CHROMA_FORMAT_UNDEFINED:
			default:
				break;
		}

		vvencYUVPlane YPlane = vvencYUVPlane();
		YPlane.ptr = Y.data();
		YPlane.width = Config.Width;
		YPlane.height = Config.Height;
		YPlane.stride = Config.Width;
		YUVInputBuffer.planes[0] = YPlane;

		vvencYUVPlane UPlane = vvencYUVPlane();
		UPlane.ptr = U.data();
		UPlane.width = ScaleX(Config.Width, Config.Format);
		UPlane.height = ScaleY(Config.Height, Config.Format);
		UPlane.stride = ScaleX(Config.Width, Config.Format);
		YUVInputBuffer.planes[1] = UPlane;

		vvencYUVPlane VPlane = vvencYUVPlane();
		VPlane.ptr = V.data();
		VPlane.width = ScaleX(Config.Width, Config.Format);
		VPlane.height = ScaleY(Config.Height, Config.Format);
		VPlane.stride = ScaleX(Config.Width, Config.Format);
		YUVInputBuffer.planes[2] = VPlane;
		SequenceNumber++;

		bool Done;
		Result = vvenc_encode(Encoder, &YUVInputBuffer, &AU, &Done);
		if (Result != VVENC_OK)
		{
			vvenc_YUVBuffer_free_buffer(&YUVInputBuffer);
			vvenc_accessUnit_free_payload(&AU);
			vvenc_encoder_close(Encoder);
			return new VvcResult(Result);
		}

		if (AU.payloadUsedSize > 0 && OnEncodedImageCallback != nullptr)
		{
			OnEncodedImageCallback->OnEncodeComplete(AU.payload, AU.payloadUsedSize);
		}
	}
	return new VvcResult(VVENC_OK);
}

int VvcEncoder::ScaleX(int InX, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
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

int VvcEncoder::ScaleY(int InY, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
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