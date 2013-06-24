#ifndef __PSYKO_AGALTRANSLATOR__
#define __PSYKO_AGALTRANSLATOR__

#include "PixelShader.h"

namespace psyko
{
	class AGALTranslator
	{
	public:
		AGALTranslator();
		~AGALTranslator();

		PixelShader Translate(const PixelShader& source);
	};
}

#endif