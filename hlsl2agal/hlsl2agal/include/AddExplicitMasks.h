#ifndef __PSYKO_ADDEXPLICITMASKS__
#define __PSYKO_ADDEXPLICITMASKS__

#include "ShaderTransformation.h"

namespace psyko
{
	class AddExplicitMasks : public ShaderTransformation
	{
	public:
		AddExplicitMasks() {}
		~AddExplicitMasks() {}

		virtual PixelShader Transform(const PixelShader& original);
	};
}

#endif