#ifndef __PSYKO_REMOVEMADTRANSFORMATION__
#define __PSYKO_REMOVEMADTRANSFORMATION__

#include "ShaderTransformation.h"

namespace psyko
{
	class RemoveMADTransformation : public ShaderTransformation
	{
	public:
		RemoveMADTransformation() {}
		~RemoveMADTransformation() {}

		virtual PixelShader Transform(const PixelShader& original);
	};
}

#endif