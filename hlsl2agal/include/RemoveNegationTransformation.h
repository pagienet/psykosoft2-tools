#ifndef __PSYKO_REMOVENEGATIONTRANSFORMATION__
#define __PSYKO_REMOVENEGATIONTRANSFORMATION__

#include "ShaderTransformation.h"

namespace psyko
{
	class RemoveNegationTransformation : public ShaderTransformation
	{
	public:
		RemoveNegationTransformation() {}
		~RemoveNegationTransformation() {}

		virtual PixelShader Transform(const PixelShader& original);
	};
}

#endif