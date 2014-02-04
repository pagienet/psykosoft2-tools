#include <iostream>
#include "lodepng.h"
#include "convert.h"

const float canvasSpecularity = 1.0f;
const float canvasGloss = 200.0f;
const float canvasBumpiness = 5.0f;

ImageData ConvertNormalSpecular(const InputData& settings, const ImageData& normals, const ImageData& specular)
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

		normalX /= canvasBumpiness;
		normalY /= canvasBumpiness;
		normalX = normalX * .5f * settings.bumpiness + .5f;
		normalY = -normalY * .5f * settings.bumpiness + .5f;	// invert Y

		// ouput as BGRA
		data.bytes[i] = specularity;
		data.bytes[i+1] = int(normalY * 255.0f);
		data.bytes[i+2] = int(normalX * 255.0f);
		data.bytes[i+3] = gloss;
	}

	return data;
}

ImageData ProcessColor(const InputData& data)
{
	ImageData inputData = LoadPNG(data.albedoFilename);
	ImageData outputData;

	outputData.width = inputData.width;
	outputData.height = inputData.height;
	unsigned int len = outputData.width*outputData.height * 4;
	outputData.bytes.resize(len);

	for (unsigned int i = 0; i < len; i += 4) {
		// convert RGBA to BGRA
		outputData.bytes[i] = inputData.bytes[i + 2];
		outputData.bytes[i + 1] = inputData.bytes[i + 1];
		outputData.bytes[i + 2] = inputData.bytes[i];
		outputData.bytes[i + 3] = inputData.bytes[i + 3];
	}

	return outputData;
}

ImageData ProcessNormalSpecular(const InputData& data)
{
	// order is RGBA, 32bits per pixel
	ImageData normalData = LoadPNG(data.normalMapFilename);
	ImageData specularData = LoadPNG(data.specularMapFilename);

	if (normalData.width != specularData.width || normalData.height != specularData.height) {
		std::cout << "Dimension mismatch!" << std::endl;
		exit(1);
	}

	return ConvertNormalSpecular(data, normalData, specularData);
}

ImageData ProcessInputData(const InputData& data)
{
	if (data.albedoFilename != "")
		return ProcessColor(data);
	else
		return ProcessNormalSpecular(data);
}