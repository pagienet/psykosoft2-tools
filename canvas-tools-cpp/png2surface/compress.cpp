#include "compress.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#define ZLIB_WINAPI
#include "zlib.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

int memread(unsigned char* dest, size_t offset, size_t count, std::vector<unsigned char>& input)
{
	unsigned char* rawInput = input.data() + offset;
	memcpy(dest, rawInput, count);
	return offset + count > input.size()? input.size() - offset : count; 
}

void memwrite(unsigned char* source, size_t count, std::vector<unsigned char>& output)
{
	for (unsigned i = 0; i < count; ++i)
		output.push_back(source[i]);
}

int CompressFile(std::vector<unsigned char>& input, std::vector<unsigned char>& output, int level)
{
	unsigned int inputIndex = 0;
	int outputIndex = 0;
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = memread(in, inputIndex, CHUNK, input);
        
		inputIndex += CHUNK;
        flush = inputIndex >= input.size() ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            memwrite(out, have, output);
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}