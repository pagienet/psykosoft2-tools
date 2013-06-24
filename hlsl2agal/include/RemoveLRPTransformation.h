#ifndef __PSYKO_REMOVELRPTRANSFORMATION__
#define __PSYKO_REMOVELRPTRANSFORMATION__

#include "ShaderTransformation.h"

namespace psyko
{
	class RemoveLRPTransformation : public ShaderTransformation
	{
	public:
		RemoveLRPTransformation() {}
		~RemoveLRPTransformation() {}

		virtual PixelShader Transform(const PixelShader& original);
	};
}

#endif