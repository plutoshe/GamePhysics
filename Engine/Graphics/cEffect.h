#pragma once

#include "cShader.h"
#include "cRenderState.h"
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
			Effect(const Effect& e) { 
				m_vertexShaderPath = e.m_vertexShaderPath; m_fragmentShaderPath = e.m_fragmentShaderPath; 
#if defined( EAE6320_PLATFORM_GL )	
				m_programId = e.m_programId;
#endif
			}
			~Effect() {}
			void SetVertexShaderPath(std::string vertexShaderPath) { m_vertexShaderPath = vertexShaderPath; }
			void SetFragmentShaderPath(std::string fragmentShaderPath) { m_fragmentShaderPath = fragmentShaderPath; }
			eae6320::cResult Load(
				eae6320::Assets::cManager<eae6320::Graphics::cShader>& manager, 
				eae6320::Graphics::cShader::Handle& vertexShader, 
				eae6320::Graphics::cShader::Handle& fragmentShader);
			eae6320::cResult Bind();

#if defined( EAE6320_PLATFORM_GL )	
			GLuint m_programId;
#endif
			eae6320::cResult Release();
		};
	}
}

