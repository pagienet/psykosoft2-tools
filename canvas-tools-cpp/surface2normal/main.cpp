#include <fstream>
#include <sstream>
#include <vector>
#include "commandline.h"
#include "lodepng.h"
#include "image.h"

struct InputData
{
	std::string inputFilename;
	std::string outputFilename;
	float bumpiness;
};

void ParseArgument(InputData* args, int* i, int argc, char* argv[])
{
	std::string argumentType = argv[(*i)++];

	if (argumentType == "-i" || argumentType == "-input")
		args->inputFilename = GetStringArg(i, argc, argv);	
	else if (argumentType == "-o" || argumentType == "-output")
		args->outputFilename = GetStringArg(i, argc, argv);		
	else if (argumentType == "-b" || argumentType == "-bumpiness")
		args->bumpiness = GetArg<float>(i, argc, argv);
}

InputData ParseArgs(int argc, char* argv[])
{
	InputData args;
	args.bumpiness = 20.0f;

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

ImageData ProcessInputData(const InputData& inputData)
{	
	ImageData source = LoadSurface(inputData.inputFilename);
	ImageData target;
	unsigned int len = source.width * source.height * 4;
	
	target.width = source.width;
	target.height = source.height;
	target.bytes.resize(len);

	for (unsigned int i = 0; i < len; i += 4) {
		unsigned char r = source.bytes[i + 2];
		unsigned char g = 0xff - source.bytes[i + 1];
		float x = (r / 128.0 - 1.0) * inputData.bumpiness;
		float y = (g / 128.0 - 1.0) * inputData.bumpiness;
		float rcpLen = 1.0f/sqrt(x*x + y*y + 1.0);
		x *= rcpLen;
		y *= rcpLen;
		float z = rcpLen;
		r = unsigned char((x * .5 + .5) * 0xff);
		g = unsigned char((y * .5 + .5) * 0xff);
		unsigned char b = unsigned char((z * .5 + .5) * 0xff);
		target.bytes[i] = r;
		target.bytes[i + 1] = g;
		target.bytes[i + 2] = b;
		target.bytes[i + 3] = 0xff;
	}

	return target;
}

int main(int argc, char* argv [])
{
	std::cout << "surface2normal.exe - using LodePNG" << std::endl;

	try {
		InputData inputData = ParseArgs(argc, argv);
		if (!IsDataValid(inputData)) return 1;

		ImageData converted = ProcessInputData(inputData);		
		SavePNG(inputData.outputFilename, converted);
	}
	catch (std::exception& error) {
		std::cout << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}