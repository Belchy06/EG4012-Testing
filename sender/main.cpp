#include "sender.h"

int main(int argc, const char* argv[])
{
	Sender Sender;

	Sender.ParseArgs(argc, argv);
	Sender.ValidateArgs();
	Sender.Test();
	// main_app.PrintEncoderSettings();
	// main_app.MainEncoderLoop();
	// main_app.PrintStatistics();

	return 0;
}