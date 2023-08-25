#include "ovb_common/common.h"
#include "ovb_send/encoders/vvc/vvc.h"

#define MAX_CODED_PICTURE_SIZE 800000

#define LogVvcEncoder "LogVvcEncoder"

vvencMsgLevel g_verbosity = VVENC_DETAILS;

void msgFnc(void*, int level, const char* fmt, va_list args)
{
	std::vfprintf(stdout, fmt, args);
}

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
	Params->m_inputBitDepth[0] = InConfig.BitDepth;
	Params->m_inputBitDepth[1] = InConfig.BitDepth;

	Params->m_internalBitDepth[0] = InConfig.BitDepth;
	Params->m_internalBitDepth[1] = InConfig.BitDepth;

	switch (InConfig.LogLevel)
	{
		case LOG_SEVERITY_SILENT:
			Params->m_verbosity = VVENC_SILENT;
			break;
		case LOG_SEVERITY_ERROR:
			Params->m_verbosity = VVENC_ERROR;
			break;
		case LOG_SEVERITY_WARNING:
			Params->m_verbosity = VVENC_WARNING;
			break;
		case LOG_SEVERITY_INFO:
			Params->m_verbosity = VVENC_INFO;
			break;
		case LOG_SEVERITY_NOTICE:
			Params->m_verbosity = VVENC_NOTICE;
			break;
		case LOG_SEVERITY_VERBOSE:
			Params->m_verbosity = VVENC_VERBOSE;
			break;
		case LOG_SEVERITY_DETAILS:
			Params->m_verbosity = VVENC_DETAILS;
			break;
	}

	switch (InConfig.Format)
	{
		case EChromaFormat::CHROMA_FORMAT_MONOCHROME:
			Params->m_internChromaFormat = VVENC_CHROMA_400;
			break;
		case EChromaFormat::CHROMA_FORMAT_420:
			Params->m_internChromaFormat = VVENC_CHROMA_420;
			break;
		case EChromaFormat::CHROMA_FORMAT_422:
			Params->m_internChromaFormat = VVENC_CHROMA_422;
			break;
		case EChromaFormat::CHROMA_FORMAT_444:
			Params->m_internChromaFormat = VVENC_CHROMA_444;
			break;
	}

	// All intra configuration
	Params->m_GOPSize = InConfig.VvcGOPSize;
	Params->m_IntraPeriod = InConfig.VvcIntraPeriod;

	Encoder = vvenc_encoder_create();

	vvenc_set_logging_callback(nullptr, msgFnc);		// register global log callback (deprecated, will be removed)
	vvenc_set_msg_callback(Params, nullptr, &::msgFnc); // register local (thread safe) logger (global logger is overwritten)

	int Result = vvenc_encoder_open(Encoder, Params);
	if (Result != VVENC_OK)
	{
		vvenc_encoder_close(Encoder);
		return new VvcResult(Result);
	}

	vvenc_get_config(Encoder, Params);

	LOG(LogVvcEncoder, LOG_SEVERITY_DETAILS, "{}", vvenc_get_config_as_string(Params, Params->m_verbosity));

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

		vvencYUVBuffer* YUVInputBufferPtr = nullptr;
		if (!bInLastPicture)
		{
			YUVInputBuffer.sequenceNumber = SequenceNumber;
			YUVInputBuffer.cts = (Params->m_TicksPerSecond > 0);
			YUVInputBuffer.ctsValid = true;
			SequenceNumber++;

			uint8_t* SrcBytes = InPictureBytes.data();
			int		 NumComponents = Config.Format == EChromaFormat::CHROMA_FORMAT_MONOCHROME ? 1 : 3;
			for (int c = 0; c < NumComponents; c++)
			{
				int Width = c == 0 ? Config.Width : ScaleX(Config.Width, Config.Format);
				int Height = c == 0 ? Config.Height : ScaleX(Config.Height, Config.Format);

				int16_t* PlanePtr = new int16_t[Width * Height]{ 0 };

				for (int y = 0; y < Height; y++)
				{
					for (int x = 0; x < Width; x++)
					{
						PlanePtr[x] = static_cast<int16_t>(SrcBytes[x]);
					}
					SrcBytes += Width;
					PlanePtr += Width;
				}

				vvencYUVPlane Plane = vvencYUVPlane();
				Plane.ptr = PlanePtr - Width * Height;
				Plane.width = Width;
				Plane.height = Height;
				Plane.stride = Width;
				YUVInputBuffer.planes[c] = std::move(Plane);
			}

			YUVInputBufferPtr = &YUVInputBuffer;
		}

		bool Done;
		Result = vvenc_encode(Encoder, YUVInputBufferPtr, &AU, &Done);
		if (Result != VVENC_OK)
		{
			vvenc_YUVBuffer_free_buffer(&YUVInputBuffer);
			vvenc_accessUnit_free_payload(&AU);
			vvenc_encoder_close(Encoder);
			return new VvcResult(Result);
		}

		if (AU.payloadUsedSize > 0 && OnEncodedImageCallback != nullptr)
		{
			std::stringstream Stream;
			Stream.write((const char*)AU.payload, AU.payloadUsedSize);

			bool Continue = true;
			while (Continue)
			{
				vvencAccessUnit Nal;
				vvenc_accessUnit_default(&Nal);
				vvenc_accessUnit_alloc_payload(&Nal, MAX_CODED_PICTURE_SIZE);
				int Res = ReadNalFromStream(&Stream, &Nal);
				Continue = Res > 0;
				if (Continue)
				{

					OnEncodedImageCallback->OnEncodeComplete(Nal.payload, Nal.payloadUsedSize);
				}
			}
		}
	}
	return new VvcResult(VVENC_OK);
}

int VvcEncoder::ReadNalFromStream(std::stringstream* InStream, vvencAccessUnit* OutAccessUnit)
{
	int Info2 = 0;
	int Info3 = 0;
	int Pos = 0;

	int			   StartCodeFound = 0;
	int			   Rewind = 0;
	uint32_t	   Len;
	unsigned char* Buf = OutAccessUnit->payload;
	OutAccessUnit->payloadUsedSize = 0;

	auto CurFillPos = InStream->tellg();
	if (CurFillPos < 0)
	{
		return -1;
	}

	// jump over possible start code
	InStream->read((char*)Buf, 5);
	size_t Extracted = InStream->gcount();
	if (Extracted < 4)
	{

		return -1;
	}

	Pos += 5;
	Info2 = 0;
	Info3 = 0;
	StartCodeFound = 0;

	while (!StartCodeFound)
	{
		if (InStream->eof())
		{
			if (Pos > 5)
			{
				Len = Pos - 1;
				OutAccessUnit->payloadUsedSize = Len;
				return Len;
			}
			else
			{
				return -1;
			}
		}

		unsigned char* P = Buf + Pos;
		InStream->read((char*)P, 1);
		Pos++;

		Info3 = RetrieveNalStartCode(&Buf[Pos - 4], 3);
		if (Info3 != 1)
		{
			Info2 = RetrieveNalStartCode(&Buf[Pos - 3], 2);
		}
		StartCodeFound = (Info2 == 1 || Info3 == 1);
	}

	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	Rewind = 0;
	if (Info3 == 1)
	{
		Rewind = -4;
	}
	else if (Info2 == 1)
	{
		Rewind = -3;
	}
	else
	{
		LOG(LogVvcEncoder, LOG_SEVERITY_ERROR, "readBitstreamFromFile: Error in next start code search");
	}

	InStream->seekg(Rewind, InStream->cur);
	if (InStream->bad() || InStream->fail())
	{
		LOG(LogVvcEncoder, LOG_SEVERITY_ERROR, "readBitstreamFromFile: Cannot seek {} in the bit stream file", Rewind);
		return -1;
	}

	// Here the Start code, the complete NALU, and the next start code is in the Buf.
	// The size of Buf is Pos, Pos+rewind are the number of bytes excluding the next
	// start code, and (Pos+rewind)-startcodeprefix_len is the size of the NALU

	Len = (Pos + Rewind);
	OutAccessUnit->payloadUsedSize = Len;
	return Len;
}

int VvcEncoder::RetrieveNalStartCode(unsigned char* pB, int InZerosInStartcode)
{
	int info = 1;
	int i = 0;
	for (i = 0; i < InZerosInStartcode; i++)
	{
		if (pB[i] != 0)
		{
			info = 0;
		}
	}

	if (pB[i] != 1)
	{
		info = 0;
	}

	return info;
}

#undef LogVvcEncoder

#undef MAX_CODED_PICTURE_SIZE