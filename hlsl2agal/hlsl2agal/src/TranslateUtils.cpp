#include "TranslateUtils.h"

#include <string>

namespace psyko
{
	static unsigned int IncreaseIfIsHigherConstant(RegisterEntry entry, unsigned int index);

	RegisterMask GenerateSourceMask(const RegisterMask& destMask)
	{
		RegisterMask sourceMask;
		char currentComponent = destMask.mask[0];

		// keep one-component stuff clean
		if (destMask.mask.size() == 1) {
			sourceMask.mask += currentComponent;
			return sourceMask;
		}
		
		unsigned int i = 1;
		
		for (unsigned int j = 0; j < 4; ++j) {
			sourceMask.mask += currentComponent;
			if ("xyzw"[j] == currentComponent) {
				if (i >= destMask.mask.size())
					i = destMask.mask.size() - 1;
				
				currentComponent = destMask.mask[i++];
			}
		}                              

		return sourceMask;
	}

	RegisterEntry GetUnusedConstantRegister(const PixelShader& shader)
	{
		unsigned int index = 0;
		for (const Statement& statement : shader.GetStatements()) {
			index = IncreaseIfIsHigherConstant(statement.source1.registerEntry, index);
			index = IncreaseIfIsHigherConstant(statement.source2.registerEntry, index);
			index = IncreaseIfIsHigherConstant(statement.source3.registerEntry, index);
		}

		RegisterEntry entry;
		entry.reg.type = RegisterType::CONSTANT;
		entry.reg.index = index;
		return entry;
	}

	unsigned int IncreaseIfIsHigherConstant(RegisterEntry entry, unsigned int index) 
	{
		if (entry.reg.type == RegisterType::CONSTANT && entry.reg.index >= index)
			index = entry.reg.index + 1;
		
		return index;
	}
}