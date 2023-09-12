#include <iterator>
#include <iostream>

#include "ovb_recv/decoders/vvc/vvc_decoder.h"
#include "ovb_recv/decoders/vvc/vvc_result.h"

#define MAX_CODED_PICTURE_SIZE 800000

#define LogVvcDecoder "LogVvcDecoder"

void msgFnc(void*, int level, const char* fmt, va_list args)
{
	std::vfprintf(stdout, fmt, args);
}

VvcDecoder::VvcDecoder()
	: Params(new vvdecParams())
	, Decoder(nullptr)
	, AccessUnit(vvdec_accessUnit_alloc())
{
}

VvcDecoder::~VvcDecoder()
{
	if (Decoder)
	{
		Decoder = nullptr;
	}

	if (Params)
	{
		Params = nullptr;
	}
}

DecodeResult* VvcDecoder::Init(DecoderConfig& InConfig)
{
	Config = InConfig;

	vvdec_params_default(Params);

	switch (InConfig.LogLevel)
	{
		case LOG_SEVERITY_SILENT:
			Params->logLevel = VVDEC_SILENT;
			break;
		case LOG_SEVERITY_ERROR:
			Params->logLevel = VVDEC_ERROR;
			break;
		case LOG_SEVERITY_WARNING:
			Params->logLevel = VVDEC_WARNING;
			break;
		case LOG_SEVERITY_INFO:
			Params->logLevel = VVDEC_INFO;
			break;
		case LOG_SEVERITY_NOTICE:
			Params->logLevel = VVDEC_NOTICE;
			break;
		case LOG_SEVERITY_VERBOSE:
			Params->logLevel = VVDEC_VERBOSE;
			break;
		case LOG_SEVERITY_DETAILS:
			Params->logLevel = VVDEC_DETAILS;
			break;
	}

	vvdec_accessUnit_alloc_payload(AccessUnit, MAX_CODED_PICTURE_SIZE);

	Params->errHandlingFlags = Config.VvcErrorHandlingFlags;

	Decoder = vvdec_decoder_open(Params);

	vvdec_set_logging_callback(Decoder, &::msgFnc);

	if (Decoder == nullptr)
	{
		vvdec_accessUnit_free(AccessUnit);
		return new VvcResult(VVDEC_ERR_INITIALIZE);
	}

	return new VvcResult(VVDEC_OK);
}

DecodeResult* VvcDecoder::Decode(uint8_t* InNalBytes, size_t InNalSize)
{
	AccessUnit->payload = reinterpret_cast<unsigned char*>(InNalBytes);
	AccessUnit->payloadSize = (int)InNalSize;
	AccessUnit->payloadUsedSize = (int)InNalSize;

	bool		 MultipleSlices = false;
	vvdecNalType NalType = vvdec_get_nal_unit_type(AccessUnit);
	std::string	 NalStr = GetNalUnitTypeAsString(NalType);
	LOG(LogVvcDecoder, LOG_SEVERITY_DETAILS, "Read Nal {}. Size {}", NalStr, AccessUnit->payloadUsedSize);

	if (NalType == VVC_NAL_UNIT_PH)
	{
		// picture header indicates multiple slices
		MultipleSlices = true;
	}

	int			 ComprPics = 0;
	vvdecNalType NalTypeSlice = VVC_NAL_UNIT_INVALID;
	bool		 IsSlice = vvdec_is_nal_unit_slice(NalType);
	if (IsSlice)
	{
		if (MultipleSlices)
		{
			if (NalTypeSlice == VVC_NAL_UNIT_INVALID)
			{
				// set current slice type and increment pic count
				ComprPics++;
				NalTypeSlice = NalType;
			}
			else
			{
				IsSlice = false; // prevent cts/dts increase if not first slice
			}
		}
		else
		{
			ComprPics++;
		}
	}

	if (NalTypeSlice != VVC_NAL_UNIT_INVALID && NalType != NalTypeSlice)
	{
		NalTypeSlice = VVC_NAL_UNIT_INVALID; // reset slice type
	}

	vvdecFrame* PcFrame = nullptr;

	int Result = vvdec_decode(Decoder, AccessUnit, &PcFrame);

	if (IsSlice)
	{
		AccessUnit->cts++;
		AccessUnit->dts++;
	}

	if (Result == VVDEC_TRY_AGAIN || Result == VVDEC_ERR_DEC_INPUT)
	{
		// Occurs when the decoder needs more NALs to produce a frame
		// Return OK to keep decoding loop running
		return new VvcResult(VVDEC_OK);
	}
	else if (Result != VVDEC_OK)
	{
		LOG(LogVvcDecoder, LOG_SEVERITY_ERROR, "{}", vvdec_get_last_error(Decoder));
		if (std::string AdditionalInfo = vvdec_get_last_additional_error(Decoder); !AdditionalInfo.empty())
		{
			LOG(LogVvcDecoder, LOG_SEVERITY_DETAILS, "{}", AdditionalInfo);
		}

		if (Result == VVDEC_ERR_RESTART_REQUIRED)
		{
			vvdec_decoder_close(Decoder);

			LOG(LogVvcDecoder, LOG_SEVERITY_NOTICE, "Recreating decoder");
			Decoder = vvdec_decoder_open(Params);

			vvdec_set_logging_callback(Decoder, &::msgFnc);
		}

		return new VvcResult(Result);
	}

	if (PcFrame != nullptr && PcFrame->ctsValid)
	{
		if (OnDecodedImageCallback != nullptr)
		{
			DecodedImage Image;

			if (PcFrame->colorFormat != VVDEC_CF_INVALID)
			{
				// clang-format off
				if       (PcFrame->colorFormat == VVDEC_CF_YUV400_PLANAR) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_400;
				} else if(PcFrame->colorFormat == VVDEC_CF_YUV420_PLANAR) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_420;
				} else if(PcFrame->colorFormat == VVDEC_CF_YUV422_PLANAR) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_422;
				} else if(PcFrame->colorFormat == VVDEC_CF_YUV444_PLANAR) {
					Image.Config.Format = EChromaFormat::CHROMA_FORMAT_444;
				}
				// clang-format on
			}

			std::vector<uint8_t> ImageBytes;
			//                                      Y
			// clang-format off
			size_t FrameSize = (PcFrame->planes[0].width * PcFrame->planes[0].height * PcFrame->planes[0].bytesPerSample)                                                            // Y
                             + (PcFrame->planes[1].width * PcFrame->planes[1].height * PcFrame->planes[1].bytesPerSample)  // U
                             + (PcFrame->planes[2].width * PcFrame->planes[2].height * PcFrame->planes[2].bytesPerSample); // V
			// clang-format on
			ImageBytes.reserve(FrameSize);

			for (size_t c = 0; c < PcFrame->numPlanes; c++)
			{
				vvdecPlane			 Plane = PcFrame->planes[c];
				std::vector<uint8_t> PlaneVec;
				PlaneVec.reserve(Plane.width * Plane.height * Plane.bytesPerSample);
				for (size_t y = 0; y < Plane.height; y++)
				{
					for (size_t x = 0; x < Plane.width; x++)
					{
						PlaneVec.push_back(Plane.ptr[x + y * Plane.width]);
					}
				}
				ImageBytes.insert(ImageBytes.end(), PlaneVec.begin(), PlaneVec.end());
			}

			Image.Bytes = ImageBytes;
			Image.Size = ImageBytes.size();
			Image.Config.BitDepth = PcFrame->bitDepth;
			Image.Config.Width = PcFrame->width;
			Image.Config.Height = PcFrame->height;

			// TODO (belchy06): Don't hardcode this
			Image.Config.FramerateNum = 50000;
			Image.Config.FramerateDenom = 1000;

			OnDecodedImageCallback->OnDecodeComplete(Image);
		}
	}

	return new VvcResult(Result);
}

std::string VvcDecoder::GetNalUnitTypeAsString(vvdecNalType InNalType)
{
	std::string NalString = "VVC_NAL_UNIT_UNIT_INVALID";

	switch (InNalType)
	{
		case VVC_NAL_UNIT_CODED_SLICE_TRAIL:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_TRAIL";
			break; // 0
		case VVC_NAL_UNIT_CODED_SLICE_STSA:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_STSA";
			break; // 1
		case VVC_NAL_UNIT_CODED_SLICE_RADL:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_RADL";
			break; // 2
		case VVC_NAL_UNIT_CODED_SLICE_RASL:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_RASL";
			break; // 3
		case VVC_NAL_UNIT_RESERVED_VCL_4:
			NalString = "VVC_NAL_UNIT_RESERVED_VCL_4";
			break; // 4
		case VVC_NAL_UNIT_RESERVED_VCL_5:
			NalString = "VVC_NAL_UNIT_RESERVED_VCL_5";
			break; // 5
		case VVC_NAL_UNIT_RESERVED_VCL_6:
			NalString = "VVC_NAL_UNIT_RESERVED_VCL_6";
			break; // 6
		case VVC_NAL_UNIT_CODED_SLICE_IDR_W_RADL:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_IDR_W_RADL";
			break; // 7
		case VVC_NAL_UNIT_CODED_SLICE_IDR_N_LP:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_IDR_N_LP";
			break; // 8
		case VVC_NAL_UNIT_CODED_SLICE_CRA:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_CRA";
			break; // 9
		case VVC_NAL_UNIT_CODED_SLICE_GDR:
			NalString = "VVC_NAL_UNIT_CODED_SLICE_GDR";
			break; // 10
		case VVC_NAL_UNIT_RESERVED_IRAP_VCL_11:
			NalString = "VVC_NAL_UNIT_RESERVED_IRAP_VCL_11";
			break; // 11
		case VVC_NAL_UNIT_RESERVED_IRAP_VCL_12:
			NalString = "VVC_NAL_UNIT_RESERVED_IRAP_VCL_12";
			break; // 12
		case VVC_NAL_UNIT_DCI:
			NalString = "VVC_NAL_UNIT_DCI";
			break; // 13
		case VVC_NAL_UNIT_VPS:
			NalString = "VVC_NAL_UNIT_VPS";
			break; // 14
		case VVC_NAL_UNIT_SPS:
			NalString = "VVC_NAL_UNIT_SPS";
			break; // 15
		case VVC_NAL_UNIT_PPS:
			NalString = "VVC_NAL_UNIT_PPS";
			break; // 16
		case VVC_NAL_UNIT_PREFIX_APS:
			NalString = "VVC_NAL_UNIT_PREFIX_APS";
			break; // 17
		case VVC_NAL_UNIT_SUFFIX_APS:
			NalString = "VVC_NAL_UNIT_SUFFIX_APS";
			break; // 18
		case VVC_NAL_UNIT_PH:
			NalString = "VVC_NAL_UNIT_PH";
			break; // 19
		case VVC_NAL_UNIT_ACCESS_UNIT_DELIMITER:
			NalString = "VVC_NAL_UNIT_ACCESS_UNIT_DELIMITER";
			break; // 20
		case VVC_NAL_UNIT_EOS:
			NalString = "VVC_NAL_UNIT_EOS";
			break; // 21
		case VVC_NAL_UNIT_EOB:
			NalString = "VVC_NAL_UNIT_EOB";
			break; // 22
		case VVC_NAL_UNIT_PREFIX_SEI:
			NalString = "VVC_NAL_UNIT_PREFIX_SEI";
			break; // 23
		case VVC_NAL_UNIT_SUFFIX_SEI:
			NalString = "VVC_NAL_UNIT_SUFFIX_SEI";
			break; // 24
		case VVC_NAL_UNIT_FD:
			NalString = "VVC_NAL_UNIT_FD";
			break; // 25
		case VVC_NAL_UNIT_RESERVED_NVCL_26:
			NalString = "VVC_NAL_UNIT_RESERVED_NVCL_26";
			break; // 26
		case VVC_NAL_UNIT_RESERVED_NVCL_27:
			NalString = "VVC_NAL_UNIT_RESERVED_NVCL_27";
			break; // 27
		case VVC_NAL_UNIT_UNSPECIFIED_28:
			NalString = "VVC_NAL_UNIT_UNSPECIFIED_28";
			break; // 28
		case VVC_NAL_UNIT_UNSPECIFIED_29:
			NalString = "VVC_NAL_UNIT_UNSPECIFIED_29";
			break; // 29
		case VVC_NAL_UNIT_UNSPECIFIED_30:
			NalString = "VVC_NAL_UNIT_UNSPECIFIED_30";
			break; // 30
		case VVC_NAL_UNIT_UNSPECIFIED_31:
			NalString = "VVC_NAL_UNIT_UNSPECIFIED_31";
			break; // 31
		case VVC_NAL_UNIT_INVALID:
		default:
			NalString = "VVC_NAL_UNIT_INVALID";
			break;
	}

	return NalString;
}

int VvcDecoder::ScaleX(int InX, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_400:
			return 0;
		case EChromaFormat::CHROMA_FORMAT_444:
			return InX;
		case EChromaFormat::CHROMA_FORMAT_420:
		case EChromaFormat::CHROMA_FORMAT_422:
			return InX << 1;
		default:
			return 0;
	}
}

int VvcDecoder::ScaleY(int InY, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case EChromaFormat::CHROMA_FORMAT_400:
			return 0;
		case EChromaFormat::CHROMA_FORMAT_444:
		case EChromaFormat::CHROMA_FORMAT_422:
			return InY;
		case EChromaFormat::CHROMA_FORMAT_420:
			return InY << 1;
		default:
			return 0;
	}
}

#undef LogVvcDecoder

#undef MAX_CODED_PICTURE_SIZE