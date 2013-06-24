#ifndef __PSYKO_SHADERWRITER__
#define __PSYKO_SHADERWRITER__

#include <ostream>
#include "PixelShader.h"

namespace psyko
{
	std::ostream& operator<<(std::ostream& stream, const PixelShader& shader);
	
	class AGALWriter
	{
	public:
		AGALWriter();
		~AGALWriter() {}

		void WriteShader(const PixelShader& shader, std::ostream& stream);

		TextureSampling::Filtering GetTextureFiltering() const { return textureFiltering; }
		void SetTextureFiltering(TextureSampling::Filtering value) { textureFiltering = value; }
		TextureSampling::MipFiltering GetMipFiltering() const { return mipFiltering; }
		void SetMipFiltering(TextureSampling::MipFiltering value) { mipFiltering = value; }
		TextureSampling::WrapMode GetWrapMode() const { return wrapMode; }
		void SetWrapMode(TextureSampling::WrapMode value) { wrapMode = value; }		

	private:
		TextureSampling::Filtering textureFiltering;
		TextureSampling::MipFiltering mipFiltering;
		TextureSampling::WrapMode wrapMode;
	};
}

#endif