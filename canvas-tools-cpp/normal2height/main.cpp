#include <fstream>
#include <iostream>
#include <sstream>
#include "commandline.h"
#include "convert.h"
#include "image.h"

void ParseArgument(InputData* args, int* i, int argc, char* argv[])
{
	std::string argumentType = argv[(*i)++];

	if (argumentType == "-i" || argumentType == "-input")
		args->inputFilename = GetStringArg(i, argc, argv);	
	else if (argumentType == "-o" || argumentType == "-output")
		args->outputFilename = GetStringArg(i, argc, argv);	
	else if (argumentType == "-q" || argumentType == "-quality")
		args->quality = GetArg<unsigned int>(i, argc, argv);
	else if (argumentType == "-r" || argumentType == "-range")
		args->range = GetArg<float>(i, argc, argv);
}

InputData ParseArgs(int argc, char* argv[])
{
	InputData args;
	args.quality = 30;
	args.range = .05f;

	int i = 0;

	while (i < argc)
		ParseArgument(&args, &i, argc, argv);

	return args;
}

bool IsDataValid(const InputData& data)
{
	if (data.outputFilename == "") {
		std::cout << "Output file not provided (usage: -o filename.png)" << std::endl;
		return false;
	}

	if (data.inputFilename == "") {
		std::cout << "Input file not provided (usage: -i filename.png)" << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char* argv [])
{
	std::cout << "normal2height.exe - using LodePNG" << std::endl;

	try {
		InputData inputData = ParseArgs(argc, argv);
		if (!IsDataValid(inputData)) return 1;

		ImageData converted = ProcessInputData(inputData, inputData.quality, inputData.range);
		
		// output 16-bits grey-scale pngs for precision
		SavePNG(inputData.outputFilename, converted, LodePNGColorType::LCT_GREY, 16);
	}
	catch (std::exception& error) {
		std::cout << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}