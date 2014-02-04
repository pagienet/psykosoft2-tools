#ifndef __PSYKO_IMAGE_H__
#define __PSYKO_IMAGE_H__

#include <vector>
#include "lodepng.h"

struct ImageData
{
	unsigned int width;
	unsigned int height;
	std::vector<unsigned char> bytes;
};

ImageData LoadPNG(std::string filename);
void SavePNG(std::string filename, const ImageData& imageData, LodePNGColorType type = LodePNGColorType::LCT_RGBA, unsigned int bitDepth = 8);

ImageData LoadSurface(std::string filename);
void SaveSurface(std::string filename, const ImageData& imageData);

#endif