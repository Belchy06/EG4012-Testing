#include "editor.h"

int main(int argc, const char* argv[])
{
	Editor Editor;

	Editor.ParseArgs(argc, argv);
	Editor.ValidateArgs();
	Editor.PrintSettings();
	Editor.Run();

	return 0;
}