#ifndef __PSYKO_COMPRESS_H__
#define __PSYKO_COMPRESS_H__

#include <vector>

int CompressFile(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, int level = 9);
int UncompressFile(const std::vector<unsigned char>& input, std::vector<unsigned char>& output);

#endif