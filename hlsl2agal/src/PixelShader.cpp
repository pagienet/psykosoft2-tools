#include "PixelShader.h"

#include <iostream>
#include <assert.h>

namespace psyko
{
	std::istream& operator>>(std::istream& stream, Opcode::Code& opcode);
	std::istream& operator>>(std::istream& stream, RegisterEntry& destination);
	std::istream& operator>>(std::istream& stream, Operand& operand);
	std::istream& operator>>(std::istream& stream, RegisterMask& mask);
	std::istream& operator>>(std::istream& stream, Register& reg);
	std::istream& operator>>(std::istream& stream, RegisterType::Type& regType);

	Opcode::Code TranslateOpcode(const std::string value);	
	RegisterType::Type TranslateRegisterType(const std::string value);	
	bool NeedsTwoSources(Opcode::Code opcode);
	bool NeedsThreeSources(Opcode::Code opcode);
	void SkipWhitespace(std::istream& stream);

	std::istream& operator>>(std::istream& stream, Statement& statement)
	{
		char ch; // used to prune commas
		stream >> statement.opcode >> statement.destination >> ch >> statement.source1;
		
		if (NeedsTwoSources(statement.opcode))
			stream >> ch >> statement.source2;

		if (NeedsThreeSources(statement.opcode))
			stream >> ch >> statement.source3;
		
		return stream;
	}

	std::istream& operator>>(std::istream& stream, Opcode::Code& opcode)
	{
		std::string code;
		SkipWhitespace(stream);

		do {
			char ch = stream.get();
			
			if (ch == ' ') {
				stream.putback(ch);
				opcode = TranslateOpcode(code);
				return stream;
			}

			code += ch;
		} while (true);

		assert(false && "Unreachable code");
		
		return stream;
	}

	std::istream& operator>>(std::istream& stream, RegisterEntry& entry)
	{
		stream >> entry.reg;
		
		if (stream.peek() == '.')
			stream >> entry.mask;
		
		return stream;
	}
	

	std::istream& operator>>(std::istream& stream, Operand& operand)
	{
		SkipWhitespace(stream);

		if (stream.peek() == '-') {
			stream.get();
			operand.negated = true;
		}

		stream >> operand.registerEntry;
		
		return stream;
	}

	std::istream& operator>>(std::istream& stream, RegisterMask& mask)
	{
		char ch;
		stream >> ch;	// remove '.'
		
		mask.mask = "";
		
		do {
			char ch = stream.get();
			
			if (ch < 'w' || ch > 'z') {
				stream.putback(ch);
				return stream;
			}

			mask.mask += ch;
		} while (true);
		
		assert(false && "Unreachable code");

		return stream;
	}

	std::istream& operator>>(std::istream& stream, Register& reg)
	{
		return stream >> reg.type >> reg.index;
	}

	std::istream& operator>>(std::istream& stream, RegisterType::Type& regType)
	{
		SkipWhitespace(stream);
		
		std::string string = "";
		
		do {
			char ch = stream.get();
			
			// until index reached
			if (ch >= '0' && ch <= '9') {
				stream.putback(ch);
				regType = TranslateRegisterType(string);
				return stream;
			}

			string += ch;
		} while (true);
		
		assert(false && "Unreachable code");

		return stream;
	}

	bool NeedsTwoSources(Opcode::Code opcode)
	{
		return opcode != Opcode::MOV && opcode != Opcode::NEG && opcode != Opcode::RCP && opcode != Opcode::RSQ && opcode != Opcode::NRM;
	}

	bool NeedsThreeSources(Opcode::Code opcode)
	{
		return opcode == Opcode::MAD || opcode == Opcode::DP2ADD || opcode == Opcode::CMP || opcode == Opcode::LRP;
	}

	Opcode::Code TranslateOpcode(const std::string value)
	{
		if (value == "add")
			return Opcode::ADD;
		else if (value == "mad")
			return Opcode::MAD;
		else if (value == "mov")
			return Opcode::MOV;
		else if (value == "mul")
			return Opcode::MUL;
		else if (value == "texld")
			return Opcode::TEXLD;
		else if (value == "max")
			return Opcode::MAX;
		else if (value == "min")
			return Opcode::MIN;
		else if (value == "dp2add")
			return Opcode::DP2ADD;
		else if (value == "dp3")
			return Opcode::DP3;
		else if (value == "dp4")
			return Opcode::DP4;
		else if (value == "cmp")
			return Opcode::CMP;
		else if (value == "rcp")
			return Opcode::RCP;
		else if (value == "rsq")
			return Opcode::RSQ;
		else if (value == "nrm")
			return Opcode::NRM;
		else if (value == "lrp")
			return Opcode::LRP;
		else {
			std::cout << "Unknown opcode " << value;
			throw std::exception("Unknown opcode");
		}
	}

	RegisterType::Type TranslateRegisterType(const std::string value)
	{
		if (value == "c")
			return RegisterType::CONSTANT;
		else if (value == "s")
			return RegisterType::STREAM;
		else if (value == "r")
			return RegisterType::TEMPORARY;
		else if (value == "v")
			return RegisterType::VARYING;
		else if (value == "oC")
			return RegisterType::OUTPUT;
		else {
			std::cout << "Unknown register type " << value;
			throw std::exception("Unknown register type");
		}
	}

	void SkipWhitespace(std::istream& stream)
	{
		char ch;
		stream >> ch;	// remove '.'
		stream.putback(ch);
	}
}