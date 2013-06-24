#ifndef __PSYKO_REMOVECMPTRANSFORMATION__
#define __PSYKO_REMOVECMPTRANSFORMATION__

#include "ShaderTransformation.h"

namespace psyko
{
	class RemoveCMPTransformation : public ShaderTransformation
	{
	public:
		RemoveCMPTransformation() {}
		~RemoveCMPTransformation() {}

		virtual PixelShader Transform(const PixelShader& original);
	};
}

#endif