#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>

#include "ovb_common/common.h"
#include "ovb_common/y4m/y4m_reader.h"
#include "ovb_common/y4m/y4m_writer.h"
#include "ovb_editor/editor.h"

#define LogEditor "LogEditor"

Editor::Editor()
{
	Options.LogLevel = LOG_SEVERITY_INFO;
}

Editor::~Editor()
{
}

void Editor::ParseArgs(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		LOG(LogEditor, LOG_SEVERITY_ERROR, "No args specified");
		PrintHelp();
		std::exit(1);
	}

	// Parse command line
	for (int i = 1; i < argc; i++)
	{
		std::string Arg(argv[i]);

		// clang-format off
		if(argc - 1 == i) {
			LOG(LogEditor, LOG_SEVERITY_ERROR, "Missing argument value: {}", Arg);
            PrintHelp();
			std::exit(1);
        } else if (Arg == "-h") {
			PrintHelp();
			std::exit(1);
		} else if(Arg == "--original") {
            std::string OriginalStr(argv[++i]);
            Options.OriginalFile = OriginalStr;
        } else if(Arg == "--distorted") {
            std::string DistortedStr(argv[++i]);
            Options.DistortedFile = DistortedStr;
        } else if(Arg == "--output") {
            std::string OutputStr(argv[++i]);
            Options.OutputFile = OutputStr;
        } else if(Arg == "--framerate") {
            std::stringstream(argv[++i]) >> Options.Framerate;
        } else if(Arg == "--method") {
            std::string MethodStr(argv[++i]);
            if(MethodStr == "dsis") {
                Options.Method = EMethod::DSIS;
            } else {
				LOG(LogEditor, LOG_SEVERITY_WARNING, "Unknown method \"{}\".", MethodStr);
                std::exit(-1);
            }
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
				LOG(LogEditor, LOG_SEVERITY_WARNING, "Unknown log level \"{}\". Defaulting to LOG_SEVERITY_INFO", LevelStr);
                Options.LogLevel = ELogSeverity::LOG_SEVERITY_INFO;
            }
        } else {
			LOG(LogEditor, LOG_SEVERITY_ERROR, "Unknown argument \"{}\"", Arg);
            PrintHelp();
            std::exit(1);
        }
		// clang-format on
	}

	OvbLogging::Verbosity = Options.LogLevel;
}
void Editor::ValidateArgs()
{
	if (Options.OriginalFile.empty())
	{
		LOG(LogEditor, LOG_SEVERITY_ERROR, "Missing \"--original\" argument");
		PrintHelp();
		std::exit(-1);
	}

	if (Options.DistortedFile.empty())
	{
		LOG(LogEditor, LOG_SEVERITY_ERROR, "Missing \"--distorted\" argument");
		PrintHelp();
		std::exit(-1);
	}

	if (Options.OutputFile.empty())
	{
		LOG(LogEditor, LOG_SEVERITY_ERROR, "Missing \"--output\" argument");
		PrintHelp();
		std::exit(-1);
	}
}

void Editor::PrintSettings()
{
	// clang-format off
	std::cout << std::endl;
    std::cout << "Running Editor:" << std::endl;
	std::cout << "  --original: " << Options.OriginalFile << std::endl;
	std::cout << "  --distorted: " << Options.DistortedFile << std::endl;
	std::cout << "  --output: " << Options.OutputFile << std::endl;
    std::cout << "  --framerate: " << Options.Framerate << std::endl;
    std::cout << "  --method: " << MethodToString(Options.Method) << std::endl;
    std::cout << "  --log-level: " << "LOG_SEVERITY_" << SeverityToString(Options.LogLevel) << std::endl;
	// clang-format on
}

void Editor::Run()
{
	if (Options.Method == DSIS)
	{
		std::ifstream OriginalStream;
		OriginalStream.open(Options.OriginalFile, std::ios_base::binary);
		if (!OriginalStream)
		{
			LOG(LogEditor, LOG_SEVERITY_ERROR, "Failed to open original file");
			std::exit(-1);
		}

		Y4mReader	   OriginalReader(&OriginalStream);
		PictureFormat  OriginalPicFormat;
		std::streamoff OriginalPictureSkip;
		if (!OriginalReader.Read(OriginalPicFormat, OriginalPictureSkip))
		{
			LOG(LogEditor, LOG_SEVERITY_ERROR, "Reading unsuccessful");
			std::exit(-1);
		}

		std::ifstream DistortedStream;
		DistortedStream.open(Options.DistortedFile, std::ios_base::binary);
		if (!DistortedStream)
		{
			LOG(LogEditor, LOG_SEVERITY_ERROR, "Failed to open distorted file");
			std::exit(-1);
		}

		Y4mReader	   DistortedReader(&DistortedStream);
		PictureFormat  DistortedPicFormat;
		std::streamoff DistortedPictureSkip;
		if (!DistortedReader.Read(DistortedPicFormat, DistortedPictureSkip))
		{
			LOG(LogEditor, LOG_SEVERITY_ERROR, "Reading unsuccessful");
			std::exit(-1);
		}

		std::filesystem::remove(Options.OutputFile);
		std::ofstream OutputStream;
		OutputStream.open(Options.OutputFile, std::ios_base::binary);
		if (!OutputStream)
		{
			LOG(LogEditor, LOG_SEVERITY_ERROR, "Failed to open output file");
			std::exit(-1);
		}

		Y4mWriter OutputWriter(&OutputStream);

		int OriginalFramesRemaining = (int)(Options.Framerate * 10);
		while (OriginalFramesRemaining >= 0)
		{
			LOG(LogEditor, LOG_SEVERITY_DETAILS, "OriginalFramesRemaining \"{}\"", OriginalFramesRemaining);
			int PictureSamples = 0;
			if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_400)
			{
				PictureSamples = OriginalPicFormat.Width * OriginalPicFormat.Height;
			}
			else if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_420)
			{
				PictureSamples = (3 * (OriginalPicFormat.Width * OriginalPicFormat.Height)) >> 1;
			}
			else if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_422)
			{
				PictureSamples = 2 * OriginalPicFormat.Width * OriginalPicFormat.Height;
			}
			else if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_444)
			{
				PictureSamples = 3 * OriginalPicFormat.Width * OriginalPicFormat.Height;
			}

			std::vector<uint8_t> PictureBytes;
			PictureBytes.resize(OriginalPicFormat.BitDepth == 8 ? PictureSamples : (PictureSamples << 1));

			if (!ReadNextPicture(&OriginalStream, OriginalPictureSkip, PictureBytes))
			{
				LOG(LogEditor, LOG_SEVERITY_ERROR, "Original file length not long enough!");
				std::exit(-1);
			}

			DecodedImage Image;
			Image.Bytes = PictureBytes;
			Image.Size = PictureBytes.size();
			Image.Config.BitDepth = OriginalPicFormat.BitDepth;
			Image.Config.Format = OriginalPicFormat.Format;
			Image.Config.Width = OriginalPicFormat.Width;
			Image.Config.Height = OriginalPicFormat.Height;
			Image.Config.FramerateNum = (int)(Options.Framerate * 1000);
			Image.Config.FramerateDenom = 1000;

			OutputWriter.WriteImageHeader(Image);
			OutputWriter.WriteImage(Image);

			OriginalFramesRemaining--;
		}

		int GreyFramesRemaining = (int)(Options.Framerate * 3);
		while (GreyFramesRemaining >= 0)
		{
			LOG(LogEditor, LOG_SEVERITY_DETAILS, "GreyFramesRemaining \"{}\"", GreyFramesRemaining);

			int PictureSamples = 0;
			if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_400)
			{
				PictureSamples = OriginalPicFormat.Width * OriginalPicFormat.Height;
			}
			else if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_420)
			{
				PictureSamples = (3 * (OriginalPicFormat.Width * OriginalPicFormat.Height)) >> 1;
			}
			else if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_422)
			{
				PictureSamples = 2 * OriginalPicFormat.Width * OriginalPicFormat.Height;
			}
			else if (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_444)
			{
				PictureSamples = 3 * OriginalPicFormat.Width * OriginalPicFormat.Height;
			}

			std::vector<uint8_t> PictureBytes;
			PictureBytes.reserve(OriginalPicFormat.BitDepth == 8 ? PictureSamples : (PictureSamples << 1));

			for (int c = 0; c < (OriginalPicFormat.Format == EChromaFormat::CHROMA_FORMAT_400 ? 1 : 3); c++)
			{
				int Width = c == 0 ? OriginalPicFormat.Width : ScaleX(OriginalPicFormat.Width, OriginalPicFormat.Format);
				int Height = c == 0 ? OriginalPicFormat.Height : ScaleY(OriginalPicFormat.Height, OriginalPicFormat.Format);

				std::vector<uint8_t> PlaneVec;
				PlaneVec.reserve(Width * Height);
				for (int y = 0; y < Height; y++)
				{
					for (int x = 0; x < Width; x++)
					{
						uint8_t Value = c == 0 ? 106 : c == 1 ? 130
															  : 126;
						PlaneVec.push_back(Value);
					}
				}
				PictureBytes.insert(PictureBytes.end(), PlaneVec.begin(), PlaneVec.end());
			}

			DecodedImage Image;
			Image.Bytes = PictureBytes;
			Image.Size = PictureBytes.size();
			Image.Config.BitDepth = OriginalPicFormat.BitDepth;
			Image.Config.Format = OriginalPicFormat.Format;
			Image.Config.Width = OriginalPicFormat.Width;
			Image.Config.Height = OriginalPicFormat.Height;
			Image.Config.FramerateNum = (int)(Options.Framerate * 1000);
			Image.Config.FramerateDenom = 1000;

			OutputWriter.WriteImageHeader(Image);
			OutputWriter.WriteImage(Image);

			GreyFramesRemaining--;
		}

		int DistortedFramesRemaining = (int)(Options.Framerate * 10);
		while (DistortedFramesRemaining >= 0)
		{
			LOG(LogEditor, LOG_SEVERITY_DETAILS, "DistortedFramesRemaining \"{}\"", DistortedFramesRemaining);

			int PictureSamples = 0;
			if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_400)
			{
				PictureSamples = DistortedPicFormat.Width * DistortedPicFormat.Height;
			}
			else if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_420)
			{
				PictureSamples = (3 * (DistortedPicFormat.Width * DistortedPicFormat.Height)) >> 1;
			}
			else if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_422)
			{
				PictureSamples = 2 * DistortedPicFormat.Width * DistortedPicFormat.Height;
			}
			else if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_444)
			{
				PictureSamples = 3 * DistortedPicFormat.Width * DistortedPicFormat.Height;
			}

			std::vector<uint8_t> PictureBytes;
			PictureBytes.resize(DistortedPicFormat.BitDepth == 8 ? PictureSamples : (PictureSamples << 1));

			if (!ReadNextPicture(&DistortedStream, DistortedPictureSkip, PictureBytes))
			{
				LOG(LogEditor, LOG_SEVERITY_ERROR, "Distorted file length not long enough!");
				std::exit(-1);
			}

			DecodedImage Image;
			Image.Bytes = PictureBytes;
			Image.Size = PictureBytes.size();
			Image.Config.BitDepth = DistortedPicFormat.BitDepth;
			Image.Config.Format = DistortedPicFormat.Format;
			Image.Config.Width = DistortedPicFormat.Width;
			Image.Config.Height = DistortedPicFormat.Height;
			Image.Config.FramerateNum = (int)(Options.Framerate * 1000);
			Image.Config.FramerateDenom = 1000;

			OutputWriter.WriteImageHeader(Image);
			OutputWriter.WriteImage(Image);

			DistortedFramesRemaining--;
		}

		GreyFramesRemaining = (int)(Options.Framerate * 3);
		while (GreyFramesRemaining >= 0)
		{
			LOG(LogEditor, LOG_SEVERITY_DETAILS, "GreyFramesRemaining \"{}\"", GreyFramesRemaining);

			int PictureSamples = 0;
			if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_400)
			{
				PictureSamples = DistortedPicFormat.Width * DistortedPicFormat.Height;
			}
			else if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_420)
			{
				PictureSamples = (3 * (DistortedPicFormat.Width * DistortedPicFormat.Height)) >> 1;
			}
			else if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_422)
			{
				PictureSamples = 2 * DistortedPicFormat.Width * DistortedPicFormat.Height;
			}
			else if (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_444)
			{
				PictureSamples = 3 * DistortedPicFormat.Width * DistortedPicFormat.Height;
			}

			std::vector<uint8_t> PictureBytes;
			PictureBytes.reserve(DistortedPicFormat.BitDepth == 8 ? PictureSamples : (PictureSamples << 1));

			for (int c = 0; c < (DistortedPicFormat.Format == EChromaFormat::CHROMA_FORMAT_400 ? 1 : 3); c++)
			{
				int Width = c == 0 ? DistortedPicFormat.Width : ScaleX(DistortedPicFormat.Width, DistortedPicFormat.Format);
				int Height = c == 0 ? DistortedPicFormat.Height : ScaleY(DistortedPicFormat.Height, DistortedPicFormat.Format);

				std::vector<uint8_t> PlaneVec;
				PlaneVec.reserve(Width * Height);
				for (int y = 0; y < Height; y++)
				{
					for (int x = 0; x < Width; x++)
					{
						uint8_t Value = c == 0 ? 106 : c == 1 ? 130
															  : 126;
						PlaneVec.push_back(Value);
					}
				}
				PictureBytes.insert(PictureBytes.end(), PlaneVec.begin(), PlaneVec.end());
			}

			DecodedImage Image;
			Image.Bytes = PictureBytes;
			Image.Size = PictureBytes.size();
			Image.Config.BitDepth = DistortedPicFormat.BitDepth;
			Image.Config.Format = DistortedPicFormat.Format;
			Image.Config.Width = DistortedPicFormat.Width;
			Image.Config.Height = DistortedPicFormat.Height;
			Image.Config.FramerateNum = (int)(Options.Framerate * 1000);
			Image.Config.FramerateDenom = 1000;

			OutputWriter.WriteImageHeader(Image);
			OutputWriter.WriteImage(Image);

			GreyFramesRemaining--;
		}

		LOG(LogEditor, LOG_SEVERITY_ERROR, "Done!");
	}
}

void Editor::PrintHelp()
{
}

bool Editor::ReadNextPicture(std::istream* InStream, std::streamoff InOffset, std::vector<uint8_t>& OutPictureBytes)
{
	if (InOffset > 0)
	{
		InStream->seekg(InOffset, std::ifstream::cur);
	}
	InStream->read(reinterpret_cast<char*>(&(OutPictureBytes)[0]), OutPictureBytes.size());
	return InStream->gcount() == static_cast<int>(OutPictureBytes.size());
}

int Editor::ScaleX(int InX, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case CHROMA_FORMAT_400:
			return 0;
		case CHROMA_FORMAT_444:
			return InX;
		case CHROMA_FORMAT_420:
		case CHROMA_FORMAT_422:
			return InX >> 1;
		default:
			return 0;
	}
}

int Editor::ScaleY(int InY, EChromaFormat InFormat)
{
	switch (InFormat)
	{
		case CHROMA_FORMAT_400:
			return 0;
		case CHROMA_FORMAT_444:
		case CHROMA_FORMAT_422:
			return InY;
		case CHROMA_FORMAT_420:
			return InY >> 1;
		default:
			return 0;
	}
}

#undef LogEditor