#include "image.h"
#include <fstream>
#include <iostream>
#include "compress.h"

ImageData LoadPNG(std::string filename)
{
	ImageData data;
	unsigned error = lodepng::decode(data.bytes, data.width, data.height, filename);

	//if there's an error, display it
	if (error) {
		std::cout << "Error decoding PNG " << filename << ": " << error << ": " << lodepng_error_text(error) << std::endl;
		exit(1);
	}

	return data;
}

std::vector<unsigned char> LoadData(const std::string& filename)
{
	std::vector<unsigned char> data;
	std::ifstream inputStream(filename, std::ios::in | std::ios::binary | std::ios::ate);
	
	if (!inputStream.is_open()) {
		std::cout << "Error: Could not open " << filename << " for reading" << std::endl;
		exit(1);
	}
	
	unsigned int len = (unsigned int) inputStream.tellg();
	data.resize(len);

	inputStream.seekg(0);
	inputStream.read(reinterpret_cast<char*>(data.data()), len);
	
	return data;
}

void SaveData(const std::string& filename, std::vector<unsigned char>& data)
{
	std::ofstream outputStream(filename, std::ios::out | std::ios::binary);
	if (!outputStream.is_open()) {
		std::cout << "Error: Could not open " << filename << " for writing" << std::endl;
		exit(1);
	}
	outputStream.write((const char*) data.data(), data.size());	
}

void SavePNG(std::string filename, const ImageData& imageData, LodePNGColorType type, unsigned int bitDepth)
{
	int error = lodepng_encode_file(filename.c_str(), imageData.bytes.data(), imageData.width, imageData.height, type, bitDepth);
	if (error) {
		std::cout << "Error encoding PNG: " << lodepng_error_text(error);
		exit(1);
	}
}

void SaveSurface(std::string filename, const ImageData& imageData)
{
	std::vector<unsigned char> output;
	int result = CompressFile(imageData.bytes, output);
	if (result != 0) {
		std::cout << "Compression error occurred" << std::endl;
		exit(1);
	}

	SaveData(filename, output);	
}

ImageData LoadSurface(std::string filename)
{
	std::vector<unsigned char> target = LoadData(filename);
	ImageData data;

	int result = UncompressFile(target, data.bytes);
	if (result != 0) {
		std::cout << "Uncompression error occurred" << std::endl;
		exit(1);
	}

	if (data.bytes.size() == 1024 * 768 * 4) {
		data.width = 1024;
		data.height = 768;
	}
	else if (data.bytes.size() == 2048 * 1536 * 4) {
		data.width = 2048;
		data.height = 1536;
	}
	else {
		std::cout << "Incorrect surface image size" << std::endl;
		exit(1);
	}
	
	return data;
}