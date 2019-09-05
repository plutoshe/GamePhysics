#pragma once

#include "cShader.h"

#include <vector>
#include <string>
namespace eae6320
{
	namespace Graphics
	{
		class Effect
		{
		public:
			std::string m_vertexShaderPath;
			std::string m_fragmentShaderPath;
			Effect() { m_vertexShaderPath = ""; m_fragmentShaderPath = ""; }
			~Effect() {}
			void SetVertexShaderPath(std::string vertexShaderPath) { m_vertexShaderPath = vertexShaderPath; }
			void SetFragmentShaderPath(std::string fragmentShaderPath) { m_fragmentShaderPath = fragmentShaderPath; }
			eae6320::cResult Load(eae6320::Assets::cManager<eae6320::Graphics::cShader>& manager, eae6320::Graphics::cShader::Handle& vertexShader, eae6320::Graphics::cShader::Handle& fragmentShader);
			void Bind(eae6320::Assets::cManager<eae6320::Graphics::cShader>& s_manager, eae6320::Graphics::cShader::Handle& s_vertexShader, eae6320::Graphics::cShader::Handle& s_fragmentShader);
		};
	}
}

