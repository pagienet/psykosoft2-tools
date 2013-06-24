#include "RemoveCMPTransformation.h"

#include <sstream>
#include "TranslateUtils.h"

namespace psyko
{
	static void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i, RegisterEntry constant);		
	static RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex);
	static RegisterEntry FindUnusedComponent(const PixelShader& shader, unsigned int statementIndex);

	PixelShader RemoveCMPTransformation::Transform(const PixelShader& original)
	{
		bool hasReplaced = false;
		PixelShader shader;
		
		unsigned int i = 0;

		// todo: parse define statements and figure out if 0 is already defined
		RegisterEntry constant = GetUnusedConstantRegister(original);

		for (const Statement& statement : original.GetStatements()) {			
			if (statement.opcode == Opcode::CMP) {
				Replace(original, &shader, statement, i, constant);
				hasReplaced = true;
			}
			else
				shader.PushStatement(statement);

			++i;
		}

		if (hasReplaced) {
			std::ostringstream stream;
			stream << original.GetInfo() << "\n// Define c" << constant.reg.index << " 0, 0, 0, 0\n";
			shader.SetInfo(stream.str());
		}
		else {
			shader.SetInfo(original.GetInfo());
		}

		return shader;
	}

	void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i, RegisterEntry constant)
	{
		RegisterEntry tempRegister;
		
		if (statement.source1.registerEntry.NumComponents() == 4)
			tempRegister = FindUnusedRegister(original, i);
		else
			tempRegister = FindUnusedComponent(original, i);

		// EXTREMELY DANGEROUS, REPLACE WITH AUTO-REG SEARCH
		RegisterEntry tempDest;
		
		if (statement.destination.reg.type == RegisterType::OUTPUT) {
			tempDest = tempRegister;
			tempDest.reg.index--;
		}
		else {
			tempDest = statement.destination;
		}

		Statement slt;
		slt.opcode = Opcode::SLT;
		slt.destination = tempRegister;
		slt.source1 = statement.source1;
		slt.source1.negated = !slt.source1.negated;	// cmp does > , so need to invert and use slt
		slt.source2.registerEntry = constant;

		// lerp with slt
		Statement sub;
		sub.opcode = Opcode::SUB;
		sub.destination = tempDest;
		sub.source1 = statement.source2;
		sub.source2 = statement.source3;
		
		Statement mul;
		mul.opcode = Opcode::MUL;
		mul.destination = tempDest;
		mul.source1.registerEntry = tempDest;
		mul.source2.registerEntry = tempRegister;

		Statement add;
		add.opcode = Opcode::ADD;
		add.destination = statement.destination;
		add.source1.registerEntry = tempDest;
		add.source2 = statement.source3;

		target->PushStatement(slt);
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