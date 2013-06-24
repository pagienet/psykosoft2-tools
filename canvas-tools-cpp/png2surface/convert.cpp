#include <iostream>
#include "lodepng.h"
#include "convert.h"

const float canvasSpecularity = 1.0f;
const float canvasGloss = 200.0f;
const float canvasBumpiness = 5.0f;

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

ImageData ProcessInputData(const InputData& data)
{
	// order is RGBA, 32bits per pixel
	ImageData normalData = LoadPNG(data.normalMapFilename);
	ImageData specularData = LoadPNG(data.specularMapFilename);

	if (normalData.width != specularData.width || normalData.height != specularData.height) {
		std::cout << "Dimension mismatch!" << std::endl;
		exit(1);
	}

	return Convert(data, normalData, specularData);
}