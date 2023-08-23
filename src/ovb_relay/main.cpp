#include "relay.h"

int main(int argc, const char* argv[])
{
	Relay Relay;

	Relay.ParseArgs(argc, argv);
	Relay.ValidateArgs();
	Relay.PrintSettings();
	Relay.Run();

	return 0;
}