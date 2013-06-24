#ifndef __PSYKO_REMOVEDP2ADDTRANSFORMATION__
#define __PSYKO_REMOVEDP2ADDTRANSFORMATION__

#include "ShaderTransformation.h"

namespace psyko
{
	class RemoveDP2ADDTransformation : public ShaderTransformation
	{
	public:
		RemoveDP2ADDTransformation() {}
		~RemoveDP2ADDTransformation() {}

		virtual PixelShader Transform(const PixelShader& original);
	};
}

#endif