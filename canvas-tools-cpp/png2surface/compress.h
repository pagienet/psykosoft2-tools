#ifndef __PSYKO_COMPRESS_H__
#define __PSYKO_COMPRESS_H__

#include <vector>

int CompressFile(std::vector<unsigned char>& input, std::vector<unsigned char>& output, int level = 9);

#endif