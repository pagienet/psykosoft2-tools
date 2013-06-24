#ifndef __PSYKO_REMOVEMASKEDOUTPUTWRITES__
#define __PSYKO_REMOVEMASKEDOUTPUTWRITES__

#include "ShaderTransformation.h"

namespace psyko
{
	class RemoveMaskedOutputWrites : public ShaderTransformation
	{
	public:
		RemoveMaskedOutputWrites() {}
		~RemoveMaskedOutputWrites() {}

		virtual PixelShader Transform(const PixelShader& original);

	private:
		RegisterEntry target;
	};
}

#endif