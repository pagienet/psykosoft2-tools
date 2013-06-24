#include "RemoveLRPTransformation.h"

namespace psyko
{
	static void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex);
	static RegisterEntry FindUnusedComponent(const PixelShader& shader, unsigned int statementIndex);

	PixelShader RemoveLRPTransformation::Transform(const PixelShader& original)
	{
		PixelShader shader;
		shader.SetInfo(original.GetInfo());
		unsigned int i = 0;

		for (const Statement& statement : original.GetStatements()) {			
			if (statement.opcode == Opcode::LRP)
				Replace(original, &shader, statement, i);
			else
				shader.PushStatement(statement);

			++i;
		}

		return shader;
	}

	void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		// dest = src3 + src1 * (src2 - src3)
		RegisterEntry tempRegister;

		if (statement.source2.registerEntry.NumComponents() == 4 || statement.source3.registerEntry.NumComponents() == 4)
			tempRegister = FindUnusedRegister(original, i);
		else
			tempRegister = FindUnusedComponent(original, i);
		
		Statement sub;
		sub.opcode = Opcode::SUB;
		sub.destination = tempRegister;
		sub.source1 = statement.source2;
		sub.source2 = statement.source3;

		Statement mul;
		mul.opcode = Opcode::MUL;
		mul.destination = tempRegister;
		mul.source1.registerEntry = tempRegister;
		mul.source2 = statement.source1;

		Statement add;
		add.opcode = Opcode::ADD;
		add.destination = statement.destination;
		add.source1 = statement.source3;
		add.source2.registerEntry = tempRegister;
		
		target->PushStatement(sub);
		target->PushStatement(mul);
		target->PushStatement(add);
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