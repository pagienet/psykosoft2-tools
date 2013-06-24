#include "AddExplicitMasks.h"

#include "TranslateUtils.h"

namespace psyko
{
	static void ReplaceTEXMask(PixelShader* target, const Statement& statement);
	static void Replace3ComponentMask(PixelShader* target, const Statement& statement);
	static void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i);	

	PixelShader AddExplicitMasks::Transform(const PixelShader& original)
	{
		PixelShader shader;
		shader.SetInfo(original.GetInfo());
		unsigned int i = 0;

		for (const Statement& statement : original.GetStatements()) {	
			if (statement.opcode == Opcode::DP4)
				shader.PushStatement(statement);
			else if (statement.opcode == Opcode::DP3 || statement.opcode == Opcode::NRM)
				Replace3ComponentMask(&shader, statement);
			else if (statement.opcode == Opcode::TEXLD && statement.source1.registerEntry.mask.IsEmpty())
				ReplaceTEXMask(&shader, statement);
			else 
				Replace(original, &shader, statement, i++);
		}

		return shader;
	}

	void ReplaceTEXMask(PixelShader* target, const Statement& statement)
	{
		Statement tex = statement;
		tex.source1.registerEntry.mask.mask = "xyxy";
		target->PushStatement(tex);
	}

	void Replace3ComponentMask(PixelShader* target, const Statement& statement)
	{
		Statement instr = statement;
		if (instr.source1.registerEntry.mask.IsEmpty())
			instr.source1.registerEntry.mask.mask = "xyz";
		if (instr.source2.registerEntry.mask.IsEmpty())
			instr.source2.registerEntry.mask.mask = "xyz";
		target->PushStatement(instr);
	}
	
	void Replace(const PixelShader& original, PixelShader* target, const Statement& statement, unsigned int i)
	{
		if (statement.destination.mask.IsEmpty()) {
			target->PushStatement(statement);		
			return;
		}

		Statement rep = statement;

		if (rep.source1.registerEntry.mask.IsEmpty())
			rep.source1.registerEntry.mask = GenerateSourceMask(rep.destination.mask);

		// Ignore source2 for single-operand and texture stuff
		if (statement.opcode != Opcode::MOV && 
			statement.opcode != Opcode::NEG &&
			statement.opcode != Opcode::TEXLD &&
			rep.source2.registerEntry.mask.IsEmpty()) {
			rep.source2.registerEntry.mask = GenerateSourceMask(rep.destination.mask);
		}
		
		target->PushStatement(rep);
	}
}