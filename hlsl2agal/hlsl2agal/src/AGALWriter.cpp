#include "AGALWriter.h"

namespace psyko
{
	std::ostream& operator<<(std::ostream& stream, Opcode::Code opcode);
	std::ostream& operator<<(std::ostream& stream, const Statement& statement);
	std::ostream& operator<<(std::ostream& stream, const Operand& operand);
	std::ostream& operator<<(std::ostream& stream, const RegisterEntry& entry);	
	std::ostream& operator<<(std::ostream& stream, const Register& reg);
	std::ostream& operator<<(std::ostream& stream, const RegisterMask& mask);
	std::ostream& operator<<(std::ostream& stream, const RegisterType::Type& regType);

	std::ostream& operator<<(std::ostream& stream, const PixelShader& shader)
	{
		unsigned int i = 0;
		stream << shader.GetInfo();
		
		for (const Statement& statement : shader.GetStatements()) {
			stream << statement << std::endl;
		}

		return stream;
	}

	AGALWriter::AGALWriter() : 
		textureFiltering(TextureSampling::LINEAR), 
		mipFiltering(TextureSampling::MIPNONE),
		wrapMode(TextureSampling::CLAMP)
	{
	}

	void AGALWriter::WriteShader(const PixelShader& shader, std::ostream& stream)
	{
		stream << shader.GetInfo();
		
		for (const Statement& statement : shader.GetStatements()) {
			stream << statement;
			if (statement.opcode == Opcode::TEXLD)
				stream	<< " <2d, " 
						<< (wrapMode == TextureSampling::CLAMP? "clamp, " : "wrap, ")
						<< (textureFiltering == TextureSampling::LINEAR? "linear, " : "nearest, ")
						<< (mipFiltering == TextureSampling::MIPNONE? "mipnone" : 
						mipFiltering == TextureSampling::MIPLINEAR? "miplinear" :
																	"mipnearest")
						<< ">";
			stream << std::endl;
		}
	}

	std::ostream& operator<<(std::ostream& stream, const Statement& statement)
	{
		stream << statement.opcode << " " << statement.destination << ", " << statement.source1;

		if (statement.HasSource2()) 
			stream << ", " << statement.source2;

		if (statement.HasSource3())
			throw std::exception("Statements with 3 source addresses not supported by AGAL");

		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, Opcode::Code opcode)
	{
		switch (opcode) {
			case Opcode::ADD:
				stream << "add";
				break;
			case Opcode::MAD:
				throw std::exception("MAD opcode not supported by AGAL");
				break;
			case Opcode::MOV:
				stream << "mov";
				break;
			case Opcode::MUL:
				stream << "mul";
				break;
			case Opcode::NEG:
				stream << "neg";
				break;
			case Opcode::SUB:
				stream << "sub";
				break;
			case Opcode::TEXLD:
				stream << "tex";
				break;
			case Opcode::MAX:
				stream << "max";
				break;
			case Opcode::MIN:
				stream << "min";
				break;
			case Opcode::DP3:
				stream << "dp3";
				break;
			case Opcode::DP4:
				stream << "dp4";
				break;
			case Opcode::SLT:
				stream << "slt";
				break;
			case Opcode::SGE:
				stream << "sge";
				break;
			case Opcode::RCP:
				stream << "rcp";
				break;
			case Opcode::RSQ:
				stream << "rsq";
				break;
			case Opcode::NRM:
				stream << "nrm";
				break;
			default:
				throw std::exception("Unknown opcode type");
		}

		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const Operand& operand)
	{
		if (operand.negated) 
			throw std::exception("Negation of operands not supported by AGAL");

		stream << operand.registerEntry;
		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const RegisterEntry& entry)
	{
		stream << entry.reg;
		if (!entry.mask.IsEmpty())
			stream << entry.mask;
		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const Register& reg)
	{
		stream << reg.type;
		if (reg.type != RegisterType::OUTPUT)
			stream << reg.index;
		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const RegisterType::Type& regType)
	{
		switch (regType) {
			case RegisterType::TEMPORARY:
				stream << "ft";
				break;
			case RegisterType::VARYING:
				stream << "v";
				break;
			case RegisterType::CONSTANT:
				stream << "fc";
				break;
			case RegisterType::STREAM:
				stream << "fs";
				break;
			case RegisterType::OUTPUT:
				stream << "oc";
				break;
			default:
				throw std::exception("Unknown register type");
		}
		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const RegisterMask& mask)
	{
		stream << "." << mask.mask;
		return stream;
	}
}