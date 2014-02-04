#include <algorithm>
#include <atomic>
#include <iostream>
#include <thread>
#include "lodepng.h"
#include "convert.h"

const float canvasSpecularity = 1.0f;
const float canvasGloss = 200.0f;
const float canvasBumpiness = 5.0f;

struct point
{
	double x, y;
};

inline bool IsInBounds(const point& pos, const ImageData& inputData)
{
	// todo: might want to improve bottom and right edges (remove - 1 and solve in interpolation)
	return pos.x >= 0 && pos.y >= 0 && pos.x < inputData.width - 2 && pos.y < inputData.height - 2;
}

inline double lerp(double a, double b, double ratio)
{
	return a + (b - a)*ratio;
}


inline point lerp(const point& a, const point& b, double ratio)
{
	return { lerp(a.x, b.x, ratio), lerp(a.y, b.y, ratio) };
}

inline point GetNormalIndexed(int index, const ImageData& inputData)
{
	return{ inputData.bytes[index] / 255.0 - .5, .5 - inputData.bytes[index + 1] / 255.0 };
}

inline point bilerp(const point& topLeft, const point& topRight, const point& bottomLeft, const point& bottomRight, const point& ratios)
{
	point top = lerp(topLeft, topRight, ratios.x);
	point bottom = lerp(bottomLeft, bottomRight, ratios.x);
	return lerp(top, bottom, ratios.y);
}

inline point SampleNormal(double x, double y, const ImageData& inputData)
{
	x += .5;
	y += .5;
	point topLeftCoord = { floor(x), floor(y) };
	point ratio = { x - topLeftCoord.x, y - topLeftCoord.y };
	
	int rowOffset = inputData.width * 4;
	int index = int(topLeftCoord.x + topLeftCoord.y * inputData.width) * 4;
	point topLeft = GetNormalIndexed(index, inputData);
	/*point topRight = GetNormalIndexed(index + 4, inputData);
	index += rowOffset;
	point bottomLeft = GetNormalIndexed(index, inputData);
	point bottomRight = GetNormalIndexed(index + 4, inputData);*/
	return topLeft; // bilerp(topLeft, topRight, bottomLeft, bottomRight, ratio);
}

double IntegrateDirection(unsigned int x, unsigned int y, const ImageData& inputData, const point& direction, unsigned int range)
{
	point pos = { x, y };
	point normal;
	double height = 0.0;
	
	unsigned int i = 0;

	for (;;) {
		pos.x += direction.x;
		pos.y += direction.y;
		if (i++ >= range || !IsInBounds(pos, inputData)) return height;
		
		normal = SampleNormal(pos.x, pos.y, inputData);
		height += normal.x * direction.x + normal.y * direction.y;
	}

	// unreachable
	return 0.0;
}

double IntegrateHeight(unsigned int x, unsigned int y, const ImageData& inputData, const std::vector<point>& directions, unsigned int range)
{		
	double height = 0.0;

	for (unsigned int i = 0; i < directions.size(); ++i) {
		height += IntegrateDirection(x, y, inputData, directions[i], range);
	}

	return height;
}

std::vector<point> InitDirections(unsigned int quality)
{
	std::vector<point> directions;
	directions.resize(quality);
	double angle = 0.0;
	double incr = 2.0 * 3.141592653 / double(quality);
	
	for (unsigned int i = 0; i < quality; ++i) {
		angle += incr;
		directions[i].x = cos(angle);
		directions[i].y = sin(angle);
	}

	return directions;
}

void EchoProgress(double ratio)
{
	const int numChars = 40;
	unsigned int len = int(ratio * numChars);
	unsigned int i = 0;
	
	for (i = 0; i < len; ++i) {
		std::cout << '=';
	}

	while (i++ < numChars) {
		std::cout << '-';
	}

	std::cout << '\r' << std::flush;
}

struct NormalMapProcess
{
	const ImageData* imageData;
	std::vector<double>* target;	
	const std::vector<point>* directions;

	unsigned int startY;
	unsigned int endY;
	unsigned int range;
	std::atomic_int* rowsComplete;
	
	bool showUpdates;

	void operator()()
	{
		double percent = 0.0;
		double oldPercent = -1.0;
		unsigned int i = startY * imageData->width;

		for (unsigned int y = startY; y < endY; ++y) {
			if (showUpdates) {
				percent = floor(double(rowsComplete->load()) / double(imageData->height) * 40) / 40;
				if (percent != oldPercent)
					EchoProgress(percent);
				oldPercent = percent;
			}

			for (unsigned int x = 0; x < imageData->width; ++x) {
				double height = IntegrateHeight(x, y, *imageData, *directions, range);
				
				(*target)[i++] = height;
			}
			
			rowsComplete->fetch_add(1);
		}
	}
};

void ProcessOnThreads(const ImageData& data, unsigned int quality, std::vector<double>& target, const std::vector<point>& directions, unsigned int range)
{
	std::atomic_int rowsComplete;
	rowsComplete.store(0);
	unsigned int numThreads = std::thread::hardware_concurrency();
	std::thread* threads = new std::thread[numThreads];
	NormalMapProcess* processes = new NormalMapProcess[numThreads];
	
	unsigned int numRowsPerThread = unsigned int(ceil(data.height / double(numThreads)));
	
	for (unsigned int i = 1; i < numThreads; ++i) {
		NormalMapProcess& process = processes[i];
		process.imageData = &data;
		process.target = &target;
		process.showUpdates = false;
		process.directions = &directions;
		process.startY = i * numRowsPerThread;
		process.endY = std::min(process.startY + numRowsPerThread, data.height);
		process.rowsComplete = &rowsComplete;
		process.range = range;
		threads[i] = std::thread(process);
	}

	NormalMapProcess process;
	process.imageData = &data;
	process.target = &target;
	process.showUpdates = true;
	process.directions = &directions;
	process.startY = 0;
	process.endY = std::min(numRowsPerThread, data.height);
	process.range = range;
	process.rowsComplete = &rowsComplete;
	process();

	for (unsigned int i = 1; i < numThreads; ++i) {
		threads[i].join();
	}
	
	delete [] processes;
	delete [] threads;
	

	EchoProgress(1);
	std::cout << std::endl;
}

ImageData ProcessInputData(const InputData& data, unsigned int quality, float range)
{
	ImageData inputData = LoadPNG(data.inputFilename);
	ImageData outputData;
	outputData.width = inputData.width;
	outputData.height = inputData.height;
	
	std::vector<point> directions = InitDirections(quality);

	unsigned int i = 0;

	std::vector<double> heights; 
	heights.resize(outputData.width * outputData.height);

	std::cout << std::endl << "Calculating height data:" << std::endl;
	
	ProcessOnThreads(inputData, quality, heights, directions, unsigned int(range * inputData.width));

	std::cout << std::endl;

	std::cout << "Normalizing:" << std::endl;

	double percent = 0.0;
	double oldPercent = -1.0;

	outputData.bytes.resize(outputData.width * outputData.height * 2);
	// normalize	

	unsigned int len = inputData.width * inputData.height;

	double maxHeight = 0.0, minHeight = 1.0;
	for (unsigned int i = 0; i < len; ++i) {
		maxHeight = std::max(maxHeight, heights[i]);
		minHeight = std::min(minHeight, heights[i]);
	}
	
	for (unsigned int i = 0; i < len; ++i) {
		percent = floor(double(i) / double(len) * 40) / 40;
		if (percent != oldPercent)
			EchoProgress(percent);
		oldPercent = percent;

		double height = (heights[i] - minHeight) / (maxHeight - minHeight);
		unsigned short data = unsigned short(height * 0xffff);
		outputData.bytes[i*2] = unsigned char(data >> 8);
		outputData.bytes[i * 2 + 1] = unsigned char(data & 0xff);
	}

	EchoProgress(1);
	std::cout << std::endl;
	std::cout << "Done!" << std::endl;

	return outputData;
}