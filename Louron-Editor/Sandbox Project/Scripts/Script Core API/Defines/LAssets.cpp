#include "LAssets.h"

#include "../Engine Callbacks.h"
#include "../Engine Util/LDebug.h"

namespace Louron
{
	namespace Assets
	{

		void MaterialUniformBlock::SetTexture(const char* name, GLSLType sampler_type, const Texture2D& texture) const
		{
			switch (sampler_type)
			{
			case GLSLType::Sampler1D: case GLSLType::Sampler1DArray: case GLSLType::Sampler1DShadow: case GLSLType::Sampler1DArrayShadow:
			case GLSLType::Sampler2D: case GLSLType::Sampler2DArray: case GLSLType::Sampler2DShadow: case GLSLType::Sampler2DArrayShadow:
			case GLSLType::Sampler3D:
			case GLSLType::SamplerCube: case GLSLType::SamplerCubeArray: case GLSLType::SamplerCubeShadow: case GLSLType::SamplerCubeArrayShadow:
			{
				ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*),
					MaterialUniformBlock_SetUniform,
					m_Handle, name, sampler_type, (void*)&texture.m_AssetHandle);
				break;
			}

			default:
			{
				Debug::Log(std::string("Invalid GLSLType for SetTexture: ") + std::to_string(static_cast<int>(sampler_type)), 2);
				break;
			}
			}
		}

	}
}