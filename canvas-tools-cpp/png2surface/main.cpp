#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "lodepng.h"
#include "zlib.h"

const float canvasSpecularity = 1.0f;
const float canvasGloss = 200.0f;
const float canvasBumpiness = 5.0f;

struct ImageData
{
	unsigned int width;
	unsigned int height;
	std::vector<unsigned char> bytes;
};

struct InputData
{
	std::string normalMapFilename;
	std::string specularMapFilename;	
	std::string outputFilename;	
	float maxSpecularity;
	float maxGloss;
	float bumpiness;
};

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

ImageData LoadPNG(std::string filename)
{
	ImageData data;
	unsigned error = lodepng::decode(data.bytes, data.width, data.height, filename);

	//if there's an error, display it
	if(error) {
		std::cout << "Error decoding PNG " << filename << ": " << error << ": " << lodepng_error_text(error) << std::endl;
		exit(1);
	}

	return data;
}

ImageData Convert(const InputData& settings, const ImageData& normals, const ImageData& specular)
{
	ImageData data;
	
	data.width = normals.width;
	data.height = normals.height;
	unsigned int len = data.width*data.height*4;
	data.bytes.resize(len);

	for (unsigned int i = 0; i < len; i += 4) {
		float normalX = normals.bytes[i] / 127.5f - 1.0f;	// mapped to 0 - 1
		float normalY = normals.bytes[i+1] / 127.5f - 1.0f;
		unsigned char specularity = int(specular.bytes[i] / canvasSpecularity * settings.maxSpecularity) & 0xff;
		unsigned char gloss = int(specular.bytes[i+1] / canvasGloss * settings.maxGloss) & 0xff;

		normalX /= canvasBumpiness*settings.bumpiness;
		normalY /= canvasBumpiness*settings.bumpiness;
		normalX = normalX * .5f + .5f;
		normalY = -normalY * .5f + .5f;	// invert Y

		// ouput as BGRA
		data.bytes[i] = specularity;
		data.bytes[i+1] = int(normalY * 255.0f);
		data.bytes[i+2] = int(normalX * 255.0f);
		data.bytes[i+3] = gloss;
	}

	return data;
}

void Process(const InputData& data)
{
	// order is RGBA, 32bits per pixel
	ImageData normalData = LoadPNG(data.normalMapFilename);
	ImageData specularData = LoadPNG(data.specularMapFilename);

	if (normalData.width != specularData.width || normalData.height != specularData.height) {
		std::cout << "Dimension mismatch!" << std::endl;
		exit(1);
	}

	ImageData converted = Convert(data, normalData, specularData);
	std::ofstream outputStream(data.outputFilename, std::ios::out | std::ios::binary );
	outputStream.write((const char*)converted.bytes.data(), converted.width*converted.height*4);
}

int main(int argc, char* argv[])
{
	std::cout << "png2surface.exe - using LodePNG" << std::endl;

	try {
		InputData inputData = ParseArgs(argc, argv);
		if (!IsDataValid(inputData)) return 1;
		Process(inputData);
	}
	catch(std::exception& error) {
		std::cout << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}