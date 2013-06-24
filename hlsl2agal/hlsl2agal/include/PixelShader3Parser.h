#ifndef __PSYKO_PIXELSHADER3PARSER__
#define __PSYKO_PIXELSHADER3PARSER__

#include <istream>
#include "PixelShader.h"

namespace psyko
{
	// todo: parse def cx, val, val, val, val statements and add them to PixelShader3 as constant definitions
	// todo: parse header and expose a lookup table for named constants/streams instead of using header as is
	// this allows us to output everything, while loading vertex + fragment programs in Flash by referencing the .agal files
	class PixelShader3Parser
	{
	public:
		PixelShader3Parser();
		~PixelShader3Parser();

		PixelShader Parse(std::istream& stream);
	};
}

#endif