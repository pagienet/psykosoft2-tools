#include "commandline.h"

std::string GetStringArg(int* i, int argc, char* argv[])
{
	if (*i == argc) {
		std::cout << "Unexpected end of arguments";
		throw std::exception();
	}

	return argv[(*i)++];
}