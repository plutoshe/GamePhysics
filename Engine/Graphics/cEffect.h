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
		class EffectAsset
		{
		public:
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(EffectAsset);
			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			using Handle = Assets::cHandle<EffectAsset>;
			static std::map<std::string, Handle> s_hanlderMap;
			static Assets::cManager<EffectAsset> s_manager;
			static eae6320::cResult Load(const std::string& i_path, EffectAsset*& o_geometry);

			eae6320::cResult InitData(std::string i_path);

			std::string m_vertexShaderPath;
			std::string m_fragmentShaderPath;

			void SetVertexShaderPath(std::string vertexShaderPath) { m_vertexShaderPath = vertexShaderPath; }
			void SetFragmentShaderPath(std::string fragmentShaderPath) { m_fragmentShaderPath = fragmentShaderPath; }


			bool m_isInitialized = false;
#if defined( EAE6320_PLATFORM_GL )	
			GLuint m_programId = 0;
#endif

			EffectAsset() { m_vertexShaderPath = ""; m_fragmentShaderPath = ""; }

			eae6320::cResult Release();
			~EffectAsset() { Release(); }
		};

		class cEffect
		{
			public:
				std::string m_path;
				EffectAsset::Handle m_handler;
				cEffect(const cEffect& i_effect) { m_path = i_effect.m_path; m_handler = i_effect.m_handler; }
				cEffect() { m_path = ""; }
				~cEffect() { }
				cEffect(std::string i_path) { m_path = i_path; }
				eae6320::cResult Load();
				eae6320::cResult PrepareShaderData();
				eae6320::cResult Prepare(
					eae6320::Assets::cManager<eae6320::Graphics::cShader>& manager);
				eae6320::cResult Bind();
		};
	}
}

