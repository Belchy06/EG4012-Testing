#include "receiver.h"

int main(int argc, const char* argv[])
{
	Receiver Receiver;

	Receiver.ParseArgs(argc, argv);
	Receiver.ValidateArgs();
	Receiver.Run();

	return 0;
}