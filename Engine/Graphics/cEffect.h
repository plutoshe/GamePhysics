#pragma once

#include "cShader.h"
#include "cRenderState.h"
#include <vector>
#include <string>
#include <Engine/Assets/ReferenceCountedAssets.h>

namespace eae6320
{
	namespace Graphics
	{
		class Effect
		{
		public:
			std::string m_vertexShaderPath;
			std::string m_fragmentShaderPath;

			void SetVertexShaderPath(std::string vertexShaderPath) { m_vertexShaderPath = vertexShaderPath; }
			void SetFragmentShaderPath(std::string fragmentShaderPath) { m_fragmentShaderPath = fragmentShaderPath; }
			eae6320::cResult LoadShaderData();
			eae6320::cResult Load(
				eae6320::Assets::cManager<eae6320::Graphics::cShader>& manager);
			eae6320::cResult Bind();
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(Effect);
			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();
			static cResult Factory(Effect*& o_effect);
			void SetToPointer(Effect* &i_effect);
			bool m_isInitialized = false;
#if defined( EAE6320_PLATFORM_GL )	
			GLuint m_programId = 0;
#endif
		private:

			Effect() { m_vertexShaderPath = ""; m_fragmentShaderPath = ""; }

			eae6320::cResult Release();
			~Effect() { Release(); }
		};
	}
}

