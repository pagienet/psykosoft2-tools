#ifndef __PSYKO_TRANSLATEUTILS_
#define __PSYKO_TRANSLATEUTILS_

#include "PixelShader.h"

namespace psyko
{
	RegisterMask GenerateSourceMask(const RegisterMask& destMask);
	
	RegisterEntry GetUnusedConstantRegister(const PixelShader& shader);
}

#endif