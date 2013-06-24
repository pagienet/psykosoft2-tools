#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "convert.h"
#include "compress.h"

float GetFloatArg(int* i, int argc, char* argv[])
{
	if (*i == argc) {
		std::cout << "Unexpected end of arguments";
		throw std::exception();
	}

	std::string str = argv[(*i)++];
	std::stringstream stream(str);
	float value = 0.0;
	stream >> value;
	return value;
}

std::string GetStringArg(int* i, int argc, char* argv[])
{
	if (*i == argc) {
		std::cout << "Unexpected end of arguments";
		throw std::exception();
	}

	return argv[(*i)++];
}

void ParseArgument(InputData* args, int* i, int argc, char* argv[])
{
	std::string argumentType = argv[(*i)++];

	if (argumentType == "-n" || argumentType == "-normalmap")
		args->normalMapFilename = GetStringArg(i, argc, argv);
	else if (argumentType == "-s" || argumentType == "-specularmap")
		args->specularMapFilename = GetStringArg(i, argc, argv);
	if (argumentType == "-o" || argumentType == "-output")
		args->outputFilename = GetStringArg(i, argc, argv);
	else if (argumentType == "-ms" || argumentType == "-maxspecularity")
		args->maxSpecularity = GetFloatArg(i, argc, argv);
	else if (argumentType == "-mg" || argumentType == "-maxgloss")
		args->maxSpecularity = GetFloatArg(i, argc, argv);
	else if (argumentType == "-b" || argumentType == "-bumpiness")
		args->bumpiness = GetFloatArg(i, argc, argv);
}

InputData ParseArgs(int argc, char* argv[])
{
	InputData args;
	args.maxSpecularity = canvasSpecularity;
	args.maxGloss = canvasGloss;
	args.bumpiness = 1.0f;

	int i = 0;
	
	while (i < argc) {
		ParseArgument(&args, &i, argc, argv);
	}

	return args;
}

bool IsDataValid(const InputData& data)
{
	if (data.normalMapFilename == "") {
		std::cout << "Normal map file not provided (usage: -n filename.png)" << std::endl;
		return false;
	}
	else if (data.specularMapFilename == "") {
		std::cout << "Specular map file not provided (usage: -s filename.png)" << std::endl;
		return false;
	}
	else if (data.outputFilename == "") {
		std::cout << "Output file not provided (usage: -o filename)" << std::endl;
		return false;
	}

	return true;
}



int main(int argc, char* argv[])
{
	std::cout << "png2surface.exe - using LodePNG" << std::endl;

	try {
		InputData inputData = ParseArgs(argc, argv);
		if (!IsDataValid(inputData)) return 1;
		
		ImageData converted = ProcessInputData(inputData);
		std::vector<unsigned char> output;
		int result = CompressFile(converted.bytes, output);
		if (result != 0) {
			std::cout << "Compression error occurred" << std::endl;
			exit(1);
		}

		std::ofstream outputStream(inputData.outputFilename, std::ios::out | std::ios::binary );
		outputStream.write((const char*)output.data(), output.size());
		
	}
	catch(std::exception& error) {
		std::cout << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}