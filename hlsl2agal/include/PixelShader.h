#ifndef __PSYKO_PIXELSHADER__
#define __PSYKO_PIXELSHADER__

#include <istream>
#include <string>
#include <vector>

namespace psyko
{
	namespace TextureSampling
	{
		enum Filtering
		{
			LINEAR,
			NEAREST			
		};

		enum MipFiltering
		{
			MIPLINEAR,
			MIPNONE,
			MIPNEAREST
		};

		enum WrapMode
		{			
			CLAMP,
			WRAP
		};
	}

	namespace Opcode
	{
		enum Code
		{
			ADD,
			SUB,
			MUL,
			MAD,
			TEXLD,
			MOV,
			NEG,
			MAX,
			MIN,
			DP2ADD,
			DP3,
			DP4,
			CMP,
			SLT,
			SGE,
			RCP,
			RSQ,
			LRP,
			NRM
		};
	};

	namespace RegisterType
	{
		enum Type
		{
			UNKNOWN,
			TEMPORARY,
			VARYING,
			CONSTANT,
			STREAM,
			OUTPUT
		};
	};

	struct Register
	{
		RegisterType::Type type;
		unsigned int index;

		Register() : type(RegisterType::UNKNOWN), index(0) {}
	};

	struct RegisterMask
	{
		std::string mask;
		RegisterMask() : mask() {}

		bool IsEmpty() const { return mask.size() == 0 || mask == "xyzw"; }
	};

	struct RegisterEntry
	{
		Register reg;
		RegisterMask mask;

		unsigned int NumComponents() const { return mask.mask.size() == 0? 4 : mask.mask.size(); }
	};

	struct Operand
	{
		RegisterEntry registerEntry;
		bool negated;

		Operand() : negated(false) {}
	};

	struct Statement
	{
		Opcode::Code opcode;
		RegisterEntry destination;
		Operand source1;
		Operand source2;
		Operand source3;	// only for fused opcodes such as mad, tex, dp2add, cmp, ...

		bool HasSource2() const { return source2.registerEntry.reg.type != RegisterType::UNKNOWN; }
		bool HasSource3() const { return source3.registerEntry.reg.type != RegisterType::UNKNOWN; }
	};

	class PixelShader
	{
	public:
		PixelShader() {}
		~PixelShader() {}

		const std::string& GetInfo() const { return info; }
		void SetInfo(const std::string& value) { info = value; }
		const std::vector<Statement>& GetStatements() const { return statements; } 
		void PushStatement(const Statement& statement) { statements.push_back(statement); } 

	private:
		std::string info;
		std::vector<Statement> statements;
	};

	std::istream& operator>>(std::istream& stream, Statement& statement);
};

#endif