#ifndef __PSYKO_CONVERT_H__
#define __PSYKO_CONVERT_H__

#include <vector>
#include "image.h"

extern const float canvasSpecularity;
extern const float canvasGloss;
extern const float canvasBumpiness;

struct InputData
{
	std::string inputFilename;
	std::string outputFilename;	
	unsigned int quality;
	float range;
};

ImageData ProcessInputData(const InputData& data, unsigned int quality, float range);

#endif