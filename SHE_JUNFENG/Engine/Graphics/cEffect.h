#pragma once

#include "cShader.h"

#include <vector>
#include <string>
namespace eae6320
{
	namespace Graphics
	{
		class ShaderEffect
		{
		public:
			std::string m_shaderPath;
			ShaderTypes::eType m_shaderType;
			ShaderEffect(std::string path, ShaderTypes::eType type) 
			{
				m_shaderPath = path;
				m_shaderType = type;
			}
		};

		class Effect
		{
		public:
			std::vector<ShaderEffect> m_shaders;
			Effect() { m_shaders.clear(); }
			~Effect() { m_shaders.clear(); }
			void AddShader(ShaderEffect shader);
			eae6320::cResult Load(eae6320::Assets::cManager<eae6320::Graphics::cShader> &manager, eae6320::Graphics::cShader::Handle &vertexShader, eae6320::Graphics::cShader::Handle &fragmentShader);
		};
	}
}

