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
	if (count > input.size() - offset) count = input.size() - offset;
	memcpy(dest, rawInput, count);	
	return offset + count > input.size()? input.size() - offset : count; 
}

void memwrite(unsigned char* source, size_t count, std::vector<unsigned char>& output)
{
	for (unsigned i = 0; i < count; ++i)
		output.push_back(source[i]);
}

int CompressFile(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, int level)
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
		strm.avail_in = memread(in, inputIndex, CHUNK, const_cast<std::vector<unsigned char>&>(input));
        
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

int UncompressFile(const std::vector<unsigned char>& input, std::vector<unsigned char>& output)
{
	unsigned int inputIndex = 0;
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = memread(in, inputIndex, CHUNK, const_cast<std::vector<unsigned char>&>(input));

		inputIndex += CHUNK;
		flush = inputIndex >= input.size() ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;
		
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void) inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			memwrite(out, have, output);			
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void) inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}