#include "RemoveMADTransformation.h"

namespace psyko
{
	static void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex);
	static RegisterEntry FindUnusedComponent(const PixelShader& shader, unsigned int statementIndex);
	

	PixelShader RemoveMADTransformation::Transform(const PixelShader& original)
	{
		PixelShader shader;
		shader.SetInfo(original.GetInfo());
		unsigned int i = 0;

		for (const Statement& statement : original.GetStatements()) {			
			if (statement.opcode == Opcode::MAD)
				Replace(original, &shader, statement, i);
			else
				shader.PushStatement(statement);

			++i;
		}

		return shader;
	}

	void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		RegisterEntry tempRegister;
		Statement mulStatement;
		Statement addStatement;

		if (statement.source1.registerEntry.NumComponents() == 4 || statement.source2.registerEntry.NumComponents() == 4)
			tempRegister = FindUnusedRegister(original, i);
		else
			tempRegister = FindUnusedComponent(original, i);

		mulStatement.opcode = Opcode::MUL;		
		mulStatement.source1 = statement.source1;		
		mulStatement.source2 = statement.source2;
		mulStatement.destination = tempRegister;
		
		addStatement.opcode = Opcode::ADD;	
		addStatement.destination = statement.destination;
		addStatement.source1 = statement.source3;
		addStatement.source2.registerEntry = tempRegister;

		if (mulStatement.source1.negated || mulStatement.source2.negated) {
			mulStatement.source1.negated = false;
			mulStatement.source2.negated = false;
			addStatement.source2.negated = true;
		}						
		
		target->PushStatement(mulStatement);
		target->PushStatement(addStatement);
	}

	// TODO: Analyse shader to find actual unused register. For now, assume 7
	// basically, this should not read from the register before it's written to
	RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex)
	{
		Register reg;
		reg.type = RegisterType::TEMPORARY;
		reg.index = 7;
		RegisterEntry entry;
		entry.reg = reg;
		return entry;
	}

	// TODO: Analyse shader to find actual unused register. For now, assume 7
	RegisterEntry FindUnusedComponent(const PixelShader& shader, unsigned int statementIndex)
	{
		RegisterEntry entry;
		entry.reg.type = RegisterType::TEMPORARY;
		entry.reg.index = 7;
		entry.mask.mask = "x";
		return entry;
	}
}