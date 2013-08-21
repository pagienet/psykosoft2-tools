#ifndef __PSYKO_CONVERT_H__
#define __PSYKO_CONVERT_H__

#include <vector>

extern const float canvasSpecularity;
extern const float canvasGloss;
extern const float canvasBumpiness;

struct ImageData
{
	unsigned int width;
	unsigned int height;
	std::vector<unsigned char> bytes;
};

struct InputData
{
	std::string albedoFilename;
	std::string normalMapFilename;
	std::string specularMapFilename;	
	std::string outputFilename;	
	float maxSpecularity;
	float maxGloss;
	float bumpiness;
};

ImageData ProcessInputData(const InputData& data);

#endif