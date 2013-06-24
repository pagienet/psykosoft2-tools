#include "FileIO.h"

#include <iostream>

namespace psyko
{
	std::vector<std::string> GetFilenamesForMask(std::string filemask)
	{
		std::vector<std::string> collection;

		WIN32_FIND_DATAA data;

		HANDLE handle = FindFirstFileA(filemask.c_str(), &data);
		if (handle == INVALID_HANDLE_VALUE) 
			return collection;
		
		do {
			collection.push_back(data.cFileName);			
		} while (FindNextFileA(handle, &data) != 0);
		
		DWORD error = GetLastError();
		if (error != ERROR_NO_MORE_FILES) 
			std::cout << "Error finding files. Error Code: " << error << std::endl;

		FindClose(handle);

		return collection;
	}
}