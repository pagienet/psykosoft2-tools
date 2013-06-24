#ifndef __PSYKO_SHADERTRANSFORMATION__
#define __PSYKO_SHADERTRANSFORMATION__

#include "PixelShader.h"

namespace psyko
{
	class ShaderTransformation
	{
	public:
		virtual PixelShader Transform(const PixelShader& original) = 0;
	};
}

#endif