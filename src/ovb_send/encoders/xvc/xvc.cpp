#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "ovb_common/common.h"
#include "xvc.h"
#include "xvc_result.h"

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
	Config = InConfig;
	Api->parameters_set_default(Params);

	if (Config.Width)
	{
		Params->width = Config.Width;
	}

	if (Config.Height)
	{
		Params->height = Config.Height;
	}

	if (Config.Framerate)
	{
		Params->framerate = Config.Framerate;
	}

	if (Config.Format != CHROMA_FORMAT_UNDEFINED)
	{
		// clang-format off
        if(Config.Format == CHROMA_FORMAT_400) {
            Params->chroma_format = XVC_ENC_CHROMA_FORMAT_MONOCHROME;
        } else if(Config.Format == CHROMA_FORMAT_420) {
            Params->chroma_format = XVC_ENC_CHROMA_FORMAT_420;
        } else if(Config.Format == CHROMA_FORMAT_422) {
            Params->chroma_format = XVC_ENC_CHROMA_FORMAT_422;
        } else if(Config.Format == CHROMA_FORMAT_444) {
            Params->chroma_format = XVC_ENC_CHROMA_FORMAT_444;
        }
		// clang-format on
	}

	// Intra only mode
	Params->num_ref_pics = InConfig.XvcNumRefPics;
	Params->max_keypic_distance = InConfig.XvcMaxKeypicDistance;
	Params->input_bitdepth = InConfig.BitDepth;
	Params->internal_bitdepth = InConfig.BitDepth;
	Params->qp = InConfig.XvcQP;

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

EncodeResult* XvcEncoder::Encode(std::vector<uint8_t>& InPictureBytes, bool bInLastPicture)
{
	xvc_enc_return_code Result;
	xvc_enc_nal_unit*	NalUnits;
	int					NumNalUnits;

	uint8_t* SrcBytes = InPictureBytes.data();

	std::vector<uint8_t*> PlanesVec;

	int NumComponents = Config.Format == EChromaFormat::CHROMA_FORMAT_400 ? 1 : 3;
	for (int c = 0; c < NumComponents; c++)
	{
		int Width = c == 0 ? Config.Width : ScaleX(Config.Width, Config.Format);
		int Height = c == 0 ? Config.Height : ScaleX(Config.Height, Config.Format);

		PlanesVec.push_back(new uint8_t[Width * Height]{ 0 });
		uint8_t* Plane = PlanesVec[c];

		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				Plane[x] = SrcBytes[x];
			}
			SrcBytes += Width;
			Plane += Width;
		}
	}

	const uint8_t* Planes[] = { PlanesVec[0], PlanesVec[1], PlanesVec[2] };

	int PlanesStride[3];
	PlanesStride[0] = Config.Width;
	PlanesStride[1] = ScaleX(Config.Width, Config.Format);
	PlanesStride[2] = ScaleX(Config.Width, Config.Format);

	if (bInLastPicture)
	{
		// Flush the encoder for remaining NalUnits and reconstructed pictures.
		Result = Api->encoder_flush(Encoder, &NalUnits, &NumNalUnits, nullptr);
		// Continue will remain true as long as there are buffered pictures
		// that should be reconstructed.
	}
	else
	{
		// Encode one picture and get 0 or 1 reconstructed picture back.
		// Also get back 0 or more NalUnits depending on if pictures are being
		// buffered in order to encode a full Sub Gop.
		Result = Api->encoder_encode2(Encoder, Planes, PlanesStride, &NalUnits, &NumNalUnits, nullptr, 0);
	}

	// Loop through all Nal Units that were received and write to file
	// the Nal Unit length followed by the actual Nal Unit.
	size_t	 CurrentSegmentBytes = 0;
	int		 CurrentSegmentPics = 0;
	int		 HighestQP = std::numeric_limits<int>::min();
	uint64_t TotalSSE = 0;
	size_t	 TotalBytes = 0;
	size_t	 MaxSegmentBytes = 0;
	int		 MaxSegmentPics = 0;
	double	 SumPsnrY = 0;
	double	 SumPsnrU = 0;
	double	 SumPsnrV = 0;
	char	 NalSize[4];

	for (int i = 0; i < NumNalUnits; i++)
	{
		NalSize[0] = NalUnits[i].size & 0xFF;
		NalSize[1] = (NalUnits[i].size >> 8) & 0xFF;
		NalSize[2] = (NalUnits[i].size >> 16) & 0xFF;
		NalSize[3] = (NalUnits[i].size >> 24) & 0xFF;
		if (NalUnits[i].stats.nal_unit_type == 16)
		{
			if (CurrentSegmentBytes > MaxSegmentBytes)
			{
				MaxSegmentBytes = CurrentSegmentBytes;
				MaxSegmentPics = CurrentSegmentPics;
			}
			CurrentSegmentBytes = 0;
			CurrentSegmentPics = -1;
		}
		else
		{
			TotalSSE += NalUnits[i].stats.sse;
			SumPsnrY += NalUnits[i].stats.psnr_y;
			SumPsnrU += NalUnits[i].stats.psnr_u;
			SumPsnrV += NalUnits[i].stats.psnr_v;
			HighestQP = std::max(HighestQP, NalUnits[i].stats.qp);
		}
		CurrentSegmentBytes += NalUnits[i].size;
		CurrentSegmentPics++;
		TotalBytes += NalUnits[i].size;

		if (OnEncodedImageCallback != nullptr)
		{
			OnEncodedImageCallback->OnEncodeComplete(NalUnits[i].bytes, NalUnits[i].size);
		}

		// Conditionally print information for each Nal Unit
		if (Config.LogLevel >= ELogSeverity::LOG_SEVERITY_DETAILS)
		{
			PrintNalInfo(NalUnits[i]);
		}
	}

	return new XvcResult(Result);
}

void XvcEncoder::PrintNalInfo(xvc_enc_nal_unit NalUnit)
{
	std::cout << "NUT:" << std::setw(6) << NalUnit.stats.nal_unit_type;
	if (NalUnit.stats.nal_unit_type < 16)
	{
		std::cout << "  POC:" << std::setw(6) << NalUnit.stats.poc;
		std::cout << "  DOC:" << std::setw(6) << NalUnit.stats.doc;
		std::cout << "  SOC:" << std::setw(6) << NalUnit.stats.soc;
		std::cout << "  TID:" << std::setw(6) << NalUnit.stats.tid;
		std::cout << "   QP:" << std::setw(6) << NalUnit.stats.qp;
	}
	else
	{
		std::cout << "     - not a picture -                          "
				  << "            ";
	}
	std::cout << "  Bytes: " << std::setw(10) << NalUnit.size;
	if (NalUnit.stats.nal_unit_type < 16)
	{
		double			  BPP = (8 * static_cast<double>(NalUnit.size) / (Config.Width * Config.Height));
		std::stringstream BPPStr;
		BPPStr << std::fixed << std::setprecision(5) << BPP;
		std::cout << "  Bpp: " << std::setw(10) << BPPStr.str();
		std::stringstream PsnrStrY;
		PsnrStrY << std::fixed << std::setprecision(3) << NalUnit.stats.psnr_y;
		std::cout << "  PSNR-Y: " << std::setw(6) << PsnrStrY.str();
		if (Params->chroma_format != XVC_ENC_CHROMA_FORMAT_MONOCHROME)
		{
			std::stringstream PsnrStrU;
			PsnrStrU << std::fixed << std::setprecision(3) << NalUnit.stats.psnr_u;
			std::cout << "  PSNR-U: " << std::setw(6) << PsnrStrU.str();
			std::stringstream PsnrStrV;
			PsnrStrV << std::fixed << std::setprecision(3) << NalUnit.stats.psnr_v;
			std::cout << "  PSNR-V: " << std::setw(6) << PsnrStrV.str();
		}
		if (NalUnit.stats.l0[0] >= 0 || NalUnit.stats.l1[0] >= 0)
		{
			std::cout << "  RefPics: L0: { ";
			int LengthL0 = sizeof(NalUnit.stats.l0) / sizeof(NalUnit.stats.l0[0]);
			for (int i = 0; i < LengthL0; i++)
			{
				if (NalUnit.stats.l0[i] > -1)
				{
					if (i > 0)
					{
						std::cout << ", ";
					}
					std::cout << std::setw(3) << NalUnit.stats.l0[i];
				}
			}
			std::cout << " } L1: { ";
			int LengthL1 = sizeof(NalUnit.stats.l1) / sizeof(NalUnit.stats.l1[0]);
			for (int i = 0; i < LengthL1; i++)
			{
				if (NalUnit.stats.l1[i] > -1)
				{
					if (i > 0)
					{
						std::cout << ", ";
					}
					std::cout << std::setw(3) << NalUnit.stats.l1[i];
				}
			}
			std::cout << " }";
		}
	}
	std::cout << std::endl;
}