#pragma once

#include <istream>
#include <vector>

#include "ovb_editor/settings.h"

class Editor
{
public:
	Editor();
	~Editor();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void PrintSettings();
	void Run();

private:
	void PrintHelp();

	bool ReadNextPicture(std::istream* InStream, std::streamoff InOffset, std::vector<uint8_t>& OutPictureBytes);
	int	 ScaleX(int InX, EChromaFormat InFormat);
	int	 ScaleY(int InY, EChromaFormat InFormat);

private:
	EditorSettings Options;

	static uint8_t MidGrey[3];
};