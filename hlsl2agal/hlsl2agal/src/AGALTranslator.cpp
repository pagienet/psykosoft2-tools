#include "AGALTranslator.h"
#include "AddExplicitMasks.h"
#include "RemoveCMPTransformation.h"
#include "RemoveDP2ADDTransformation.h"
#include "RemoveLRPTransformation.h"
#include "RemoveMADTransformation.h"
#include "RemoveNegationTransformation.h"
#include "RemoveMaskedOutputWrites.h"

#include <iostream>

namespace psyko
{	
	AGALTranslator::AGALTranslator()
	{
	}
	
	AGALTranslator::~AGALTranslator()
	{
	}

	PixelShader AGALTranslator::Translate(const PixelShader& original)
	{
		RemoveMADTransformation removeMAD;
		RemoveCMPTransformation removeCMP;
		RemoveLRPTransformation removeLRP;
		RemoveNegationTransformation removeNegation;
		RemoveMaskedOutputWrites removeOutputWrites;
		AddExplicitMasks addExplicitMasks;
		RemoveDP2ADDTransformation removeDP2ADD;

		PixelShader shader;				
		shader.SetInfo(original.GetInfo());
		shader = removeMAD.Transform(original);
		shader = removeLRP.Transform(shader);
		shader = removeCMP.Transform(shader);
		shader = removeNegation.Transform(shader);
		shader = removeDP2ADD.Transform(shader);
		shader = removeOutputWrites.Transform(shader);
		shader = addExplicitMasks.Transform(shader);		
		return shader;
	}
}