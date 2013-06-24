#ifndef __PSYKO_FILEIO__
#define __PSYKO_FILEIO__

#ifdef WIN32
#include <Windows.h>
#endif

#include <string>
#include <vector>

namespace psyko
{
#ifdef WIN32
	std::vector<std::string> GetFilenamesForMask(std::string filemask);
#endif
}

#endif