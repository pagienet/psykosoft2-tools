#include "RemoveMaskedOutputWrites.h"

namespace psyko
{
	static void Replace(PixelShader* target, const Statement& statement, RegisterEntry& targetReg);
	static RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex);
	static RegisterEntry FindUnusedComponent(const PixelShader& shader, unsigned int statementIndex);

	PixelShader RemoveMaskedOutputWrites::Transform(const PixelShader& original)
	{
		PixelShader shader;
		shader.SetInfo(original.GetInfo());
		unsigned int i = 0;

		target.reg.type = RegisterType::UNKNOWN;

		for (const Statement& statement : original.GetStatements()) {			
			if (statement.destination.reg.type == RegisterType::OUTPUT && !statement.destination.mask.IsEmpty()) {
				if (target.reg.type == RegisterType::UNKNOWN)
					target = FindUnusedRegister(original, i);
				Replace(&shader, statement, target);
			}
			else
				shader.PushStatement(statement);

			++i;
		}

		if (target.reg.type != RegisterType::UNKNOWN) {
			Statement mov;
			mov.opcode = Opcode::MOV;
			mov.destination.reg.type = RegisterType::OUTPUT;
			mov.source1.registerEntry = target;
			shader.PushStatement(mov);
		}

		return shader;
	}

	void Replace(PixelShader* target, const Statement& statement, RegisterEntry& targetReg)
	{
		Statement diverted = statement;
		diverted.destination = targetReg;
		diverted.destination.mask = statement.destination.mask;
		target->PushStatement(diverted);
	}

	// TODO: Analyse shader to find actual unused register. For now, assume 6
	// basically, this should not read from the register before it's written to
	RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex)
	{
		Register reg;
		reg.type = RegisterType::TEMPORARY;
		reg.index = 6;
		RegisterEntry entry;
		entry.reg = reg;
		return entry;
	}
}