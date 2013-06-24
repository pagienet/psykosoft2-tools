#ifndef __PSYKO_COMMANDLINEARGUMENTS__
#define __PSYKO_COMMANDLINEARGUMENTS__

#include <string>

namespace psyko
{
	class CommandLineArguments
	{
	public:
		enum Status {
			OK,
			NOT_ENOUGH_ARGS,
			MISSING_ARGS,
			INPUT_FILE_IS_OUTPUT_FILE
		};

		CommandLineArguments(int argc, char* argv[]);
		~CommandLineArguments();

		Status GetStatus() const { return status; }
		const std::string& GetErrorTag() const { return errorTag; }

		const std::string& GetInputFilename() const { return inputFilename; }
		bool Verbose() const { return verbose; }
		bool YesToAll() const { return yesToAll; }

	private:
		Status status;
		std::string errorTag;
		std::string inputFilename;
		bool verbose;
		bool yesToAll;
		
		void InitOptionalParams(int argc, char* argv[]);
		std::string GetString(int index, int argc, char* argv[], std::string tag);
	};
}

#endif