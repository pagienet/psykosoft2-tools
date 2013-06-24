#ifndef __PSYKO_COMPRESS_H__
#define __PSYKO_COMPRESS_H__

#include <vector>


/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int CompressFile(std::vector<unsigned char>& input, std::vector<unsigned char>& output, int level = 9);

#endif