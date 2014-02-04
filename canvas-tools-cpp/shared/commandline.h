#ifndef __PSYKO_COMMANDLINE__
#define __PSYKO_COMMANDLINE__

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

template<class T>
T GetArg(int* i, int argc, char* argv[])
{
	if (*i == argc) {
		std::cout << "Unexpected end of arguments";
		throw std::exception();
	}

	std::string str = argv[(*i)++];
	std::stringstream stream(str);
	T value = 0;
	stream >> value;
	return value;
}

std::string GetStringArg(int* i, int argc, char* argv []);

#endif