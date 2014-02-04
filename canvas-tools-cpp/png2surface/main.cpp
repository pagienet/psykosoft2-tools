#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "convert.h"
#include "commandline.h"
#include "image.h"


void ParseArgument(InputData* args, int* i, int argc, char* argv[])
{
	std::string argumentType = argv[(*i)++];

	if (argumentType == "-a" || argumentType == "-albedomap")
		args->albedoFilename = GetStringArg(i, argc, argv);
	else if (argumentType == "-n" || argumentType == "-normalmap")
		args->normalMapFilename = GetStringArg(i, argc, argv);
	else if (argumentType == "-s" || argumentType == "-specularmap")
		args->specularMapFilename = GetStringArg(i, argc, argv);
	if (argumentType == "-o" || argumentType == "-output")
		args->outputFilename = GetStringArg(i, argc, argv);
	else if (argumentType == "-ms" || argumentType == "-maxspecularity")
		args->maxSpecularity = GetArg<float>(i, argc, argv);
	else if (argumentType == "-mg" || argumentType == "-maxgloss")
		args->maxGloss = GetArg<float>(i, argc, argv);
	else if (argumentType == "-b" || argumentType == "-bumpiness")
		args->bumpiness = GetArg<float>(i, argc, argv);
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
	if (data.outputFilename == "") {
		std::cout << "Output file not provided (usage: -o filename)" << std::endl;
		return false;
	}

	if (data.albedoFilename != "") {
		if (data.normalMapFilename != "" || data.specularMapFilename != "") {
			std::cout << "Cannot provide normal map and/or specular map when providing albedo map!" << std::endl;
			return false;
		}
		else
			return true;
	}
	else if (data.normalMapFilename == "" && data.specularMapFilename == "") {
		std::cout << "Neither albedo map nor specular maps are provided (usage: -a filename.png, or -n filename1.png -s filename2.png)" << std::endl;
		return false;
	}

	if (data.normalMapFilename == "") {
		std::cout << "Normal map file not provided (usage: -n filename.png)" << std::endl;
		return false;
	}
	else if (data.specularMapFilename == "") {
		std::cout << "Specular map file not provided (usage: -s filename.png)" << std::endl;
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
		SaveSurface(inputData.outputFilename, converted);
	}
	catch(std::exception& error) {
		std::cout << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}