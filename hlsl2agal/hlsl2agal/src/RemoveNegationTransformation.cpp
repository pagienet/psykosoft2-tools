#include "RemoveNegationTransformation.h"
#include "TranslateUtils.h"

namespace psyko
{
	static void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static void RemoveNegationFromSUB(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static void RemoveNegationFromADD(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static void RemoveNegationFromMUL(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static void RemoveNegationFromMOV(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);
	static void ExtractNegation(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);

	RegisterEntry FindUnusedRegister(const PixelShader& shader, unsigned int statementIndex);
	RegisterEntry FindUnusedComponent(const PixelShader& shader, unsigned int statementIndex);


	PixelShader RemoveNegationTransformation::Transform(const PixelShader& original)
	{
		PixelShader shader;
		shader.SetInfo(original.GetInfo());
		unsigned int i = 0;

		for (const Statement& statement : original.GetStatements()) {			
			if (statement.source1.negated || statement.source2.negated) {
				Replace(original, &shader, statement, i);
			}
			else
				shader.PushStatement(statement);

			++i;
		}

		return shader;
	}

	void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		if (statement.opcode == Opcode::ADD)
			RemoveNegationFromADD(original, target, statement, i);
		else if (statement.opcode == Opcode::SUB)
			RemoveNegationFromSUB(original, target, statement, i);
		else if (statement.opcode == Opcode::MOV)
			RemoveNegationFromMOV(original, target, statement, i);
		else if (statement.opcode == Opcode::MUL)
			RemoveNegationFromMUL(original, target, statement, i);
		else 
			ExtractNegation(original, target, statement, i);
	}

	void RemoveNegationFromSUB(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		if (statement.source1.negated && statement.source2.negated) {
			Statement sub;
			sub.opcode = Opcode::SUB;
			sub.destination = statement.destination;
			sub.source1 = statement.source2;
			sub.source2 = statement.source1;
			sub.source1.negated = false;
			sub.source2.negated = false;
			target->PushStatement(sub);
		}
		else if (statement.source1.negated){
			// (- a - b) --> -(a + b)
			Statement add;
			add.opcode = Opcode::ADD;
			add.destination = statement.destination;
			add.source1 = statement.source1;
			add.source2 = statement.source2;
			add.source1.negated = false;
			add.source2.negated = false;
			Statement neg;
			neg.opcode = Opcode::NEG;
			neg.destination = statement.destination;
			neg.source1.registerEntry = statement.destination;
			target->PushStatement(add);
			target->PushStatement(neg);
		}
		else if (statement.source2.negated) {
			// (a - -b) --> a + b
			Statement add;
			add.opcode = Opcode::ADD;
			add.destination = statement.destination;
			add.source1 = statement.source1;
			add.source2 = statement.source2;
			add.source1.negated = false;
			add.source2.negated = false;			
			target->PushStatement(add);
		}
	}

	void RemoveNegationFromADD(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		if (statement.source1.negated && statement.source2.negated) {
			Statement add;
			add.opcode = Opcode::SUB;
			add.destination = statement.destination;
			add.source1 = statement.source1;
			add.source2 = statement.source2;
			add.source1.negated = false;
			add.source2.negated = false;
			Statement neg;
			neg.opcode = Opcode::NEG;
			neg.destination = statement.destination;
			neg.source1.registerEntry = statement.destination;
			target->PushStatement(add);
			target->PushStatement(neg);
		}
		else {
			Statement sub;
			sub.opcode = Opcode::SUB;
			sub.destination = statement.destination;
			if (statement.source1.negated) {
				sub.source1 = statement.source2;
				sub.source2 = statement.source1;
			}
			else {
				sub.source1 = statement.source1;
				sub.source2 = statement.source2;
			}
			sub.source1.negated = false;
			sub.source2.negated = false;			
			target->PushStatement(sub);
		}
	}

	void RemoveNegationFromMOV(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		if (statement.source1.registerEntry.reg.type == RegisterType::CONSTANT) {
			Statement mov;
			mov.opcode = Opcode::MOV;
			mov.destination = statement.destination;
			mov.source1.registerEntry = statement.source1.registerEntry;
			Statement neg;
			neg.opcode = Opcode::NEG;
			neg.destination = statement.destination;		
			neg.source1.registerEntry = statement.destination;
			neg.source1.registerEntry.mask = GenerateSourceMask(statement.destination.mask);
			target->PushStatement(mov);
			target->PushStatement(neg);
		}
		else {
			// it would kind of be weird to run into a mov statement like this
			Statement rep = statement;
			rep.source1.negated = false;
			rep.opcode = Opcode::NEG;
			target->PushStatement(rep);
		}
	}

	void RemoveNegationFromMUL(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		Statement mul;
		mul.opcode = Opcode::MUL;
		mul.destination = statement.destination;
		mul.source1 = statement.source2;
		mul.source2 = statement.source1;
		mul.source1.negated = false;
		mul.source2.negated = false;			
		target->PushStatement(mul);

		Statement neg;
		neg.opcode = Opcode::NEG;
		neg.destination = statement.destination;
		neg.source1.registerEntry = statement.destination;
		target->PushStatement(neg);
	}

	void ExtractNegation(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		Statement rep = statement;

		if (rep.source1.negated && rep.source2.negated)
			throw std::exception("Negating both operands currently not supported");
		
		Statement neg;
		RegisterEntry tempRegister;

		if (statement.source1.registerEntry.NumComponents() == 4 || statement.source2.registerEntry.NumComponents() == 4)
			tempRegister = FindUnusedRegister(original, i);
		else
			tempRegister = FindUnusedComponent(original, i);

		RegisterEntry sourceToNegate = rep.source1.negated? rep.source1.registerEntry : rep.source2.registerEntry;
		
		neg.opcode = Opcode::NEG;
		neg.destination = tempRegister;		
		neg.source1.registerEntry = sourceToNegate;

		if (rep.source1.negated) {
			rep.source1.negated = false;
			rep.source1.registerEntry = tempRegister;
		}
		else {
			rep.source2.negated = false;
			rep.source2.registerEntry = tempRegister;
		}
		target->PushStatement(neg);
		target->PushStatement(rep);
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