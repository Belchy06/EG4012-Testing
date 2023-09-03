#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "ovb_common/common.h"
#include "ovb_send/encoders/encoder.h"
#include "ovb_send/encoders/encoder_config.h"
#include "ovb_send/sender.h"

#define LogSender "LogSender"

Sender::Sender()
	: InputStream(nullptr)
	, FrameCount(0)
{
	Config.Format = EChromaFormat::CHROMA_FORMAT_UNDEFINED;
}

Sender::~Sender()
{
}

void Sender::ParseArgs(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		LOG(LogSender, LOG_SEVERITY_ERROR, "No args specified");
		PrintHelp();
		std::exit(1);
	}

	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		std::string Arg(argv[i]);

		// clang-format off
		if(argc - 1 == i) {
            LOG(LogSender, LOG_SEVERITY_ERROR, "Missing argument value: {}", Arg);
            PrintHelp();
			std::exit(1);
        } else if (Arg == "-h") {
			PrintHelp();
			std::exit(1);
		} else if(Arg == "--ip") {
            std::stringstream(argv[++i]) >> Options.IP;
        } else if(Arg == "--port") {
            std::stringstream(argv[++i]) >> Options.Port;
        } else if(Arg == "--codec") {
            std::string CodecStr(argv[++i]);
            if(CodecStr == "VVC") {
                Options.Codec = ECodec::CODEC_VVC;
            } else if(CodecStr == "XVC") {
                Options.Codec = ECodec::CODEC_XVC;
            } else if(CodecStr == "OVC") {
                Options.Codec = ECodec::CODEC_OVC;
            } else if(CodecStr == "AVC") {
                Options.Codec = ECodec::CODEC_AVC;
            } else {
                Options.Codec = ECodec::CODEC_UNDEFINED;
            }
        } else if(Arg == "--file") {
            Options.File = std::string(argv[++i]);
        } else if(Arg == "--log-level") {
            std::string LevelStr(argv[++i]);
            if(LevelStr == "silent") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            } else if(LevelStr == "error") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_ERROR;
            } else if(LevelStr == "warning") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_WARNING;
            } else if(LevelStr == "info") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            } else if(LevelStr == "notice") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_NOTICE;
            } else if(LevelStr == "verbose") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_VERBOSE;
            } else if(LevelStr == "details") {
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_DETAILS;
            } else {
                LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown log level \"{}\". Defaulting to LOG_SEVERITY_INFO", LevelStr);
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            }
        } else if(Arg == "--encoder-config") {
            std::istringstream Stream(argv[++i]);
            std::string Option;
            while(std::getline(Stream, Option, ','))
            {
                std::string Key = Option.substr(0, Option.find("="));
                std::string Value = Option.substr(Option.find("=") + 1);
                if(Key == "--format") {
                    if(Value == "400") {
                        Config.Format = EChromaFormat::CHROMA_FORMAT_400;
                    } else if(Value == "420") {
                        Config.Format = EChromaFormat::CHROMA_FORMAT_420;
                    } else if(Value == "422") {
                        Config.Format = EChromaFormat::CHROMA_FORMAT_422;
                    } else if(Value == "444") {
                        Config.Format = EChromaFormat::CHROMA_FORMAT_444;
                    } else {
                        LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown format \"{}\". Defaulting to what's specified in the source file", Value);
                    }

                } else if(Key == "--ovc-bits-per-pixel") {
                    std::stringstream(Value) >> Config.OvcBitsPerPixel;
                } else if(Key == "--ovc-repeat-vps") {
                    std::stringstream(Value) >> Config.OvcRepeatVPS;
                } else if(Key == "--ovc-num-parts-exp") {
                    std::stringstream(Value) >> Config.OvcNumPartsExp;
                } else if(Key == "--ovc-num-levels") {
                    std::stringstream(Value) >> Config.OvcNumLevels;
                } else if(Key == "--ovc-wavelet-family") {
                    if(Value == "skip") {
                        Config.OvcWaveletFamily = OVC_WAVELET_FAMILY_SKIP;
                    } else if(Value == "biorthogonal") {
                        Config.OvcWaveletFamily = OVC_WAVELET_FAMILY_BIORTHOGONAL;
                    } else if(Value == "coiflets") {
                        Config.OvcWaveletFamily = OVC_WAVELET_FAMILY_COIFLETS;
                    } else if(Value == "daubechies") {
                        Config.OvcWaveletFamily = OVC_WAVELET_FAMILY_DAUBECHIES;
                    } else if(Value == "haar") {
                        Config.OvcWaveletFamily = OVC_WAVELET_FAMILY_HAAR;
                    } else if(Value == "reverse-biorthogonal") {
                        Config.OvcWaveletFamily = OVC_WAVELET_FAMILY_REVERSE_BIORTHOGONAL;
                    } else if(Value == "symlets") {
                        Config.OvcWaveletFamily = OVC_WAVELET_FAMILY_SYMLETS;
                    } else {
                        LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown ovc_wavelet_family \"{}\"", Value);
                    }
                } else if(Key == "--ovc-wavelet-config") {
                    Config.OvcWaveletConfig.value = atoi(Value.c_str());
                } else if(Key == "--ovc-partition-type") {
                    if(Value == "skip") {
                        Config.OvcPartitionType = OVC_PARTITION_SKIP;
                    } else if(Value == "offset-zerotree") {
                        Config.OvcPartitionType = OVC_PARTITION_OFFSET_ZEROTREE;
                    } else if(Value == "zertotree-preserving") {
                        Config.OvcPartitionType = OVC_PARTITION_ZEROTREE_PRESERVING;
                    } else {
                        LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown ovc_partition_type \"{}\"", Value);
                    }
                } else if(Key == "--ovc-spiht") {
                    if(Value == "skip") {
                        Config.OvcSPIHT = OVC_SPIHT_SKIP;
                    } else if(Value == "enable") {
                        Config.OvcSPIHT = OVC_SPIHT_ENABLE;
                    } else {
                        LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown ovc_spiht \"{}\"", Value);
                    }
                } else if(Key == "--ovc-entropy-coder") {
                    if(Value == "skip") {
                        Config.OvcEntropyCoder = OVC_ENTROPY_CODER_SKIP;
                    } else if(Value == "arithmetic") {
                        Config.OvcEntropyCoder = OVC_ENTROPY_CODER_ARITHMETIC;
                    } else if(Value == "huffman") {
                        Config.OvcEntropyCoder = OVC_ENTROPY_CODER_HUFFMAN;
                    } else {
                        LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown ovc_entropy_coder \"{}\"", Value);
                    }
                } else if(Key == "--ovc-interleaver") {
                    if(Value == "skip") {
                        Config.OvcInterleaver = OVC_INTERLEAVE_SKIP;
                    } else if(Value == "random") {
                        Config.OvcInterleaver = OVC_INTERLEAVE_RANDOM;
                    } else {
                        LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown ovc_spiht \"{}\"", Value);
                    }
                } else if(Key == "--vvc-gop-size") {
                    std::stringstream(Value) >> Config.VvcGOPSize;
                } else if(Key == "--vvc-intra-period") {
                    std::stringstream(Value) >> Config.VvcIntraPeriod;
                } else if(Key == "--xvc-num-ref-pics") {
                    std::stringstream(Value) >> Config.XvcNumRefPics;
                } else if(Key == "--xvc-max-key-pic-distance") {
                    std::stringstream(Value) >> Config.XvcMaxKeypicDistance;
                } else if(Key == "--xvc-qp") {
                    std::stringstream(Value) >> Config.XvcQP;
                } else if(Key == "--avc-target-bitrate") {
                    std::stringstream(Value) >> Config.AvcTargetBitrate;
                } else {
                    LOG(LogSender, LOG_SEVERITY_WARNING, "Unknown encoder config option \"{}\"", Key);
                }
            }
        } else {
            LOG(LogSender, LOG_SEVERITY_ERROR, "Unknown argument \"{}\"", Arg);
            PrintHelp();
            std::exit(1);
        }
		// clang-format on
	}

	OvbLogging::Verbosity = Options.LogLevel;

	SocketConfig Config;
	Config.IP = Options.IP;
	Config.Port = Options.Port;

	RtpSender = RTPSender::Create();
	RtpSender->Init(Config);
}

void Sender::ValidateArgs()
{
	if (Options.File.empty())
	{
		LOG(LogSender, LOG_SEVERITY_ERROR, "Missing input file argument");
		std::exit(-1);
	}

	FileStream.open(Options.File, std::ios_base::binary);
	if (!FileStream)
	{
		LOG(LogSender, LOG_SEVERITY_ERROR, "Failed to open file");
		std::exit(-1);
	}
	InputStream = &FileStream;
	InputStream->seekg(0, std::ifstream::beg);

	if (Options.Codec == ECodec::CODEC_UNDEFINED)
	{
		LOG(LogSender, LOG_SEVERITY_ERROR, "Invalid codec");
		std::exit(-1);
	}
}

void Sender::Run()
{
	do
	{
		LOG(LogSender, LOG_SEVERITY_SILENT, "Press a key to continue...");
	}
	while (std::cin.get() != '\n');

	WrappedEncoder = EncoderFactory::Create(Options.Codec);
	Packetizer = PacketizerFactory::Create(Options.Codec);

	Y4mReader	  Reader(InputStream);
	PictureFormat PicFormat;
	if (!Reader.Read(PicFormat, PictureSkip))
	{
		LOG(LogSender, LOG_SEVERITY_ERROR, "Reading unsuccessful");
		std::exit(-1);
	}

	Config.Width = PicFormat.Width;
	Config.Height = PicFormat.Height;
	Config.BitDepth = PicFormat.BitDepth;
	Config.Framerate = PicFormat.Framerate;
	if (Config.Format == EChromaFormat::CHROMA_FORMAT_UNDEFINED)
	{
		Config.Format = PicFormat.Format;
	}
	Config.LogLevel = Options.LogLevel;

	EncodeResult* Result = WrappedEncoder->Init(Config);
	if (!Result->IsSuccess())
	{
		LOG(LogSender, LOG_SEVERITY_ERROR, "Erorr intializing config");
		std::exit(-1);
	}

	WrappedEncoder->RegisterEncodeCompleteCallback(this);

	int PictureSamples = 0;
	if (PicFormat.Format == EChromaFormat::CHROMA_FORMAT_400)
	{
		PictureSamples = Config.Width * Config.Height;
	}
	else if (PicFormat.Format == EChromaFormat::CHROMA_FORMAT_420)
	{
		PictureSamples = (3 * (Config.Width * Config.Height)) >> 1;
	}
	else if (PicFormat.Format == EChromaFormat::CHROMA_FORMAT_422)
	{
		PictureSamples = 2 * Config.Width * Config.Height;
	}
	else if (PicFormat.Format == EChromaFormat::CHROMA_FORMAT_444)
	{
		PictureSamples = 3 * Config.Width * Config.Height;
	}

	std::vector<uint8_t> PictureBytes;
	PictureBytes.resize(Config.BitDepth == 8 ? PictureSamples : (PictureSamples << 1));

	bool bContinue = true;
	while (bContinue)
	{
		bContinue = ReadNextPicture(InputStream, PictureBytes);
		bool bLastPic = !bContinue;

		LOG(LogSender, LOG_SEVERITY_INFO, "Encoding Frame {}", FrameCount++);
		Result = WrappedEncoder->Encode(PictureBytes, bLastPic);
		if (!Result->IsSuccess())
		{
			LOG(LogSender, LOG_SEVERITY_ERROR, "{}", Result->Error());
			std::exit(-1);
		}
	}
}

void Sender::OnEncodeComplete(std::vector<NALU> InNALUs)
{
	LOG(LogSender, LOG_SEVERITY_NOTICE, "OnEncodeComplete: Size {}", InNALUs.size());

	Packetizer->Packetize(InNALUs);
	std::vector<RTPPacket> Packets = Packetizer->Flush();

	std::string SizeStr = "[ ";
	for (RTPPacket& Packet : Packets)
	{
		std::stringstream ss;
		ss << Packet.GetPayloadSize() << " ";
		SizeStr += ss.str();
	}
	SizeStr += "]";
	LOG(LogSender, LOG_SEVERITY_NOTICE, "Sending {} packets. Size {}", Packets.size(), SizeStr);
	RtpSender->Send(Packets);
}

bool Sender::ReadNextPicture(std::istream* InStream, std::vector<uint8_t>& OutPictureBytes)
{
	if (PictureSkip > 0)
	{
		InStream->seekg(PictureSkip, std::ifstream::cur);
	}
	InStream->read(reinterpret_cast<char*>(&(OutPictureBytes)[0]), OutPictureBytes.size());
	return InStream->gcount() == static_cast<int>(OutPictureBytes.size());
}

void Sender::PrintHelp()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
	std::cout << "  --file <string> [Optional parameters]" << std::endl << std::endl;
    std::cout << "Optional parameters:" << std::endl;
    std::cout << "  --ip <string>       (default: \"127.0.0.1\")" << std::endl;
    std::cout << "  --port <int>        (default: 8888)" << std::endl;
    std::cout << "  --log-level <string> " << std::endl;
    std::cout << "      \"silent\"          " << std::endl;
    std::cout << "      \"error\"           " << std::endl;
    std::cout << "      \"warning\"         " << std::endl;
    std::cout << "      \"info\"            " << std::endl;
    std::cout << "      \"notice\"          " << std::endl;
    std::cout << "      \"verbose\"         " << std::endl;
    std::cout << "      \"details\"         " << std::endl;
    std::cout << "  --codec <string>        " << std::endl;
    std::cout << "      \"AVC\"             " << std::endl;
    std::cout << "      \"OVC\"             " << std::endl;
    std::cout << "      \"VVC\"             " << std::endl;
    std::cout << "      \"XVC\"             " << std::endl;
	// clang-format on
}

void Sender::PrintSettings()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Running Sender:" << std::endl;
	std::cout << "  --file: " << Options.File << std::endl;
	std::cout << "  --ip: " << Options.IP << std::endl;
	std::cout << "  --port: " << Options.Port << std::endl;
    std::cout << "  --codec: " << CodecToString(Options.Codec) << std::endl;
    std::cout << "  --log-level: " << "LOG_SEVERITY_" << SeverityToString(Options.LogLevel) << std::endl;
    std::cout << "  --encoder-config: " << std::endl;
    std::cout << "    --format: " << FormatToString(Config.Format) << std::endl;
    if(Options.Codec == CODEC_OVC) {
    std::cout << "    --ovc-bits-per-pixel: " << Config.OvcBitsPerPixel << std::endl;
    std::cout << "    --ovc-repeat-vps: " << (Config.OvcRepeatVPS ? "true" : "false") << std::endl;
    std::cout << "    --ovc-num-parts-exp: " << Config.OvcNumPartsExp << std::endl;
    std::cout << "    --ovc-num-levels: " << Config.OvcNumLevels << std::endl;
    std::cout << "    --ovc-wavelet-family: " << wavelet_family_to_string(Config.OvcWaveletFamily) << std::endl;
    if(Config.OvcWaveletFamily == OVC_WAVELET_FAMILY_BIORTHOGONAL) {
        std::cout << "    --ovc-wavelet-config: " << biorthogonal_config_to_string(Config.OvcWaveletConfig.biorthogonal_config) << std::endl;
    } else if(Config.OvcWaveletFamily == OVC_WAVELET_FAMILY_COIFLETS) {
        std::cout << "    --ovc-wavelet-config: " << coiflets_config_to_string(Config.OvcWaveletConfig.coiflets_config) << std::endl;
    } else if(Config.OvcWaveletFamily == OVC_WAVELET_FAMILY_DAUBECHIES) {
        std::cout << "    --ovc-wavelet-config: " << daubechies_config_to_string(Config.OvcWaveletConfig.daubechies_config) << std::endl;
    } else if(Config.OvcWaveletFamily == OVC_WAVELET_FAMILY_HAAR) {
        std::cout << "    --ovc-wavelet-config: " << haar_config_to_string(Config.OvcWaveletConfig.haar_config) << std::endl;
    } else if(Config.OvcWaveletFamily == OVC_WAVELET_FAMILY_REVERSE_BIORTHOGONAL) {
        std::cout << "    --ovc-wavelet-config: " << reverse_biorthogonal_config_to_string(Config.OvcWaveletConfig.reverse_biorthogonal_config) << std::endl;
    } else if(Config.OvcWaveletFamily == OVC_WAVELET_FAMILY_SYMLETS) {
        std::cout << "    --ovc-wavelet-config: " << symlets_config_to_string(Config.OvcWaveletConfig.symlets_config) << std::endl;
    }
    std::cout << "    --ovc-partition-type: " << partition_to_string(Config.OvcPartitionType) << std::endl;
    std::cout << "    --ovc-spiht: " << spiht_to_string(Config.OvcSPIHT) << std::endl;
    std::cout << "    --ovc-entropy-coder: " << entropy_coder_to_string(Config.OvcEntropyCoder) << std::endl;
    std::cout << "    --ovc-interleaver: " << interleave_to_string(Config.OvcInterleaver) << std::endl;
    } else if(Options.Codec == CODEC_VVC) {
    std::cout << "    --vvc-gop-size: " << Config.VvcGOPSize << std::endl;
    std::cout << "    --vvc-intra-period: " << Config.VvcIntraPeriod << std::endl;
    } else if(Options.Codec == CODEC_XVC) {
    std::cout << "    --xvc-num-ref-pics: " << Config.XvcNumRefPics << std::endl;
    std::cout << "    --xvc-max-key-pic-distance: " << Config.XvcMaxKeypicDistance << std::endl;
    std::cout << "    --xvc-qp: " << Config.XvcQP << std::endl;
    } else if(Options.Codec == CODEC_AVC) {
    std::cout << "    --avc-target-bitrate: " << Config.AvcTargetBitrate << std::endl;
    }
	// clang-format on
}

#undef LogSender