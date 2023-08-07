#include "sender.h"

int main(int argc, const char* argv[])
{
	CSender Sender;

	Sender.ParseArgs(argc, argv);
	Sender.ValidateArgs();
	// main_app.PrintEncoderSettings();
	// main_app.MainEncoderLoop();
	// main_app.PrintStatistics();

	return 0;
}