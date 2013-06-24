#include "AGALTranslator.h"
#include "AGALWriter.h"
#include "CommandLineArguments.h"
#include "FileIO.h"
#include "PixelShader.h"
#include "PixelShader3Parser.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

void Translate(std::istream& input, std::ostream& output, bool verbose)
{
	psyko::PixelShader3Parser parser;
	psyko::PixelShader shader = parser.Parse(input);
	psyko::AGALTranslator translator;
	psyko::AGALWriter writer;
	shader = translator.Translate(shader);

	if (verbose)
		writer.WriteShader(shader, std::cout);

	writer.WriteShader(shader, output);	
}

void DisplayUsageDescription()
{
	std::cout	<< "Usage: ps3agal inputfilename [-v] [-y]" << std::endl
				<< std::endl 
				<< "   -v:(Optional) Verbose: output generated code to the command line." << std::endl
				<< "   -y:(Optional) Yes to all: overwrites files without prompting the user." << std::endl;
}

std::string CommandLineStatusDescription(const psyko::CommandLineArguments& args)
{
	switch (args.GetStatus()) {
		case psyko::CommandLineArguments::NOT_ENOUGH_ARGS:
			return "Invalid number of parameters";
		case psyko::CommandLineArguments::MISSING_ARGS:
			return "Parameter missing after " + args.GetErrorTag();
		case psyko::CommandLineArguments::INPUT_FILE_IS_OUTPUT_FILE:
			return "Cannot use the input file as output file";
		default:
			return "Unknown error in command line parameters";
	}
}

bool FileExists(std::string filename)
{
	std::ifstream file(filename);
	return file.is_open();
}

bool UserDeniedOverwrite(std::string filename)
{
	std::string str;

	while (true) {
		std::cout << "File \"" << filename << "\" already exists. Overwrite? (y/n): ";
		std::cin >> str;		
		std::transform(str.begin(), str.end(), str.begin(), tolower);

		if (str == "y" || str == "yes") {
			return false;
		}
		else if (str == "n" || str == "no") {
			return true;
		}

		std::cout << "Unknown input." << std::endl;
	}
}

std::string GetOutputFilename(const std::string& inputFilename)
{
	int index = inputFilename.find_last_of(".");
	return inputFilename.substr(0, index) + ".agal";
}

int TranslateFile(const std::string& inputFilename, const psyko::CommandLineArguments& args)
{
	std::ifstream inputFile(inputFilename);
	
	std::string outputFilename = GetOutputFilename(inputFilename);

	if (FileExists(outputFilename) && (!args.YesToAll() && UserDeniedOverwrite(outputFilename))) {
		std::cout << "Operation aborted." << std::endl;
		return 0;
	}
		
	std::ofstream outputFile(outputFilename);

	if (!inputFile) {
		std::cout << "Failed to open file \"" << inputFilename << "\" for reading" << std::endl;
		return 1;
	}

	if (!outputFile) {
		std::cout << "Failed to open file \"" << outputFilename << "\" for writing" << std::endl;
		return 1;
	}

	try {
		Translate(inputFile, outputFile, args.Verbose());
	}
	catch(std::exception& exception)
	{
		std::cout << exception.what() << std::endl;
		return 1;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	psyko::CommandLineArguments args(argc, argv);

	if (args.GetStatus() != psyko::CommandLineArguments::OK) {
		std::cout << "Error: " << CommandLineStatusDescription(args) << std::endl << std::endl;
		DisplayUsageDescription();
		return 1;
	}

	std::vector<std::string> filenames = psyko::GetFilenamesForMask(args.GetInputFilename());
	
	if (filenames.size() == 0) {
		std::cout << "No files found" << std::endl;
		return 0;
	}

	for (const std::string& filename : filenames) {
		std::cout << "Translating " << filename << "..." << std::endl;
		TranslateFile(filename, args);			
	}

	return 0;
}