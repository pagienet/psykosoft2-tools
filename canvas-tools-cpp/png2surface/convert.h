#ifndef __PSYKO_CONVERT_H__
#define __PSYKO_CONVERT_H__

#include "image.h"

extern const float canvasSpecularity;
extern const float canvasGloss;
extern const float canvasBumpiness;

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