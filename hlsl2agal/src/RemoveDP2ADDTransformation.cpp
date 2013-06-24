#include "RemoveDP2ADDTransformation.h"

namespace psyko
{
	static void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex);
	std::string GetMaskXY(const RegisterMask& mask);

	PixelShader RemoveDP2ADDTransformation::Transform(const PixelShader& original)
	{
		PixelShader shader;
		shader.SetInfo(original.GetInfo());
		unsigned int i = 0;

		for (const Statement& statement : original.GetStatements()) {			
			if (statement.opcode == Opcode::DP2ADD)
				Replace(original, &shader, statement, i);
			else
				shader.PushStatement(statement);

			++i;
		}

		return shader;
	}

	void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		RegisterEntry tempRegister = FindUnusedRegister(original, i);
		
		std::string mask1 = GetMaskXY(statement.source1.registerEntry.mask);
		std::string mask2 = GetMaskXY(statement.source2.registerEntry.mask);

		Statement mul1;
		Statement mul2;
		Statement add1;
		Statement add2;

		mul1.opcode = Opcode::MUL;
		mul1.destination = tempRegister;
		mul1.destination.mask.mask = "x";
		mul1.source1 = statement.source1;
		mul1.source1.registerEntry.mask.mask = mask1[0];
		mul1.source2 = statement.source2;
		mul1.source2.registerEntry.mask.mask = mask2[0];
		
		mul2.opcode = Opcode::MUL;
		mul2.destination = tempRegister;
		mul2.destination.mask.mask = "y";
		mul2.source1 = statement.source1;
		mul2.source1.registerEntry.mask.mask = mask1[1];
		mul2.source2 = statement.source2;
		mul2.source2.registerEntry.mask.mask = mask2[1];


		add1.opcode = Opcode::ADD;
		add1.destination = tempRegister;
		add1.destination.mask.mask = "x";
		add1.source1.registerEntry = tempRegister;
		add1.source1.registerEntry.mask.mask = "x";
		add1.source2.registerEntry = tempRegister;
		add1.source2.registerEntry.mask.mask = "y";

		add2.opcode = Opcode::ADD;
		add2.destination = statement.destination;
		add2.source1.registerEntry = tempRegister;
		add2.source1.registerEntry.mask.mask = "x";
		add2.source2 = statement.source3;
	
		target->PushStatement(mul1);
		target->PushStatement(mul2);
		target->PushStatement(add1);
		target->PushStatement(add2);
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

	std::string GetMaskXY(const RegisterMask& mask)
	{
		if (mask.IsEmpty())
			return "xy";
		else if (mask.mask.size() == 1) {
			return mask.mask + mask.mask[0];
		}
		else 
			return mask.mask.substr(0, 2);
	}

}