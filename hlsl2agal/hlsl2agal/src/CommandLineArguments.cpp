#include "CommandLineArguments.h"

namespace psyko
{
	CommandLineArguments::CommandLineArguments(int argc, char* argv[]) :
		status(OK),
		verbose(false),
		yesToAll(false)
	{
		if (argc < 2) {
			status = NOT_ENOUGH_ARGS;
			return;
		}

		inputFilename = argv[1];
		
		InitOptionalParams(argc, argv);
	}
	
	CommandLineArguments::~CommandLineArguments()
	{
	}

	void CommandLineArguments::InitOptionalParams(int argc, char* argv[])
	{		
	 	for (int i = 2; i < argc; ++i) {
			std::string tag = argv[i];
			
			if (tag == "-v")
				verbose = true;

			if (tag == "-y")
				yesToAll = true;

			if (errorTag != "") return;
		}
	}

	std::string CommandLineArguments::GetString(int index, int argc, char* argv[], std::string tag)
	{
		if (index >= argc || argv[index][0] == '-') {
			status = MISSING_ARGS;
			errorTag = tag;
			return "";
		}

		return argv[index];
	}
}