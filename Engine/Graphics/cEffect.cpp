#include "GraphicsEnv.h"
#include "cEffect.h"
#include "PlatformIncludes.h"
#include <map>
#include <Engine/Asserts/Asserts.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Platform/Platform.h>


eae6320::Assets::cManager<eae6320::Graphics::EffectAsset> eae6320::Graphics::EffectAsset::s_manager;

std::map<std::string, eae6320::Graphics::EffectAsset::Handle> eae6320::Graphics::EffectAsset::s_hanlderMap = std::map<std::string, eae6320::Graphics::EffectAsset::Handle>();


eae6320::cResult eae6320::Graphics::cEffect::Bind()
{
	EffectAsset* effect = EffectAsset::s_manager.Get(m_handler);
	auto result = eae6320::Results::Success;
#if defined( EAE6320_PLATFORM_D3D )
	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);
	constexpr ID3D11ClassInstance* const* noInterfaces = nullptr;
	constexpr unsigned int interfaceCount = 0;
	// Vertex shader
	{
		if (eae6320::Graphics::Env::s_vertexShaders.find(effect->m_vertexShaderPath) == eae6320::Graphics::Env::s_vertexShaders.end())
		{
			EAE6320_ASSERTF(false, "Can't load vertex shader");
			return eae6320::Results::Failure;
		}
		else {
			EAE6320_ASSERT(eae6320::Graphics::Env::s_vertexShaders[effect->m_vertexShaderPath]);
			auto* const shader = cShader::s_manager.Get(eae6320::Graphics::Env::s_vertexShaders[effect->m_vertexShaderPath]);
			EAE6320_ASSERT(shader && shader->m_shaderObject.vertex);
			direct3dImmediateContext->VSSetShader(shader->m_shaderObject.vertex, noInterfaces, interfaceCount);
		}
	}
	// Fragment shader
	{
		if (eae6320::Graphics::Env::s_fragmentShaders.find(effect->m_fragmentShaderPath) == eae6320::Graphics::Env::s_fragmentShaders.end())
		{
			EAE6320_ASSERTF(false, "Can't load vertex shader");
			return eae6320::Results::Failure;
		}
		else {
			EAE6320_ASSERT(eae6320::Graphics::Env::s_fragmentShaders[effect->m_fragmentShaderPath]);
			auto* const shader = cShader::s_manager.Get(eae6320::Graphics::Env::s_fragmentShaders[effect->m_fragmentShaderPath]);
			EAE6320_ASSERT(shader && shader->m_shaderObject.fragment);
			direct3dImmediateContext->PSSetShader(shader->m_shaderObject.fragment, noInterfaces, interfaceCount);
		}
	}

#elif defined( EAE6320_PLATFORM_GL )

	EAE6320_ASSERT(effect->m_programId != 0);
	glUseProgram(effect->m_programId);
	EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
#endif
	// Render state
	{
		EAE6320_ASSERT(eae6320::Graphics::Env::s_renderState);
		auto* const renderState = cRenderState::s_manager.Get(eae6320::Graphics::Env::s_renderState);
		EAE6320_ASSERT(renderState);
		renderState->Bind();
	}
	return result;
}

eae6320::cResult eae6320::Graphics::cEffect::PrepareShaderData()
{
	EffectAsset* effect = EffectAsset::s_manager.Get(m_handler);
	auto result = eae6320::Results::Success;
	if (!(result = eae6320::Graphics::LoadShaderData(
		effect->m_vertexShaderPath,
		eae6320::Graphics::Env::s_vertexShaders,
		eae6320::Graphics::ShaderTypes::Vertex)) ||
		!(result = eae6320::Graphics::LoadShaderData(
			effect->m_fragmentShaderPath,
			eae6320::Graphics::Env::s_fragmentShaders,
			eae6320::Graphics::ShaderTypes::Fragment)))
	{
		EAE6320_ASSERTF(false, "Can't initialize shader for effect");
	}
	return result;
}


eae6320::cResult eae6320::Graphics::cEffect::Load()
{
	auto result = eae6320::Results::Success;
	if (m_path == "")
	{
		EAE6320_ASSERTF(false, "Load Geometry failed");
		return  eae6320::Results::Failure;
	}
	if (EffectAsset::s_hanlderMap.find(m_path) != EffectAsset::s_hanlderMap.end())
	{
		m_handler = EffectAsset::s_hanlderMap[m_path];
	}
	else if (!(result = EffectAsset::s_manager.Load(
		m_path,
		m_handler)))
	{
		EAE6320_ASSERTF(false, "Load Geometry failed");
		return result;
	}
	else {
		EffectAsset::s_hanlderMap[m_path] = m_handler;
	}

	return result;
}

eae6320::cResult eae6320::Graphics::EffectAsset::Load(const std::string& i_path, EffectAsset*& o_effect)
{
	auto result = Results::Success;

	EffectAsset* newEffect = nullptr;
	cScopeGuard scopeGuard([&o_effect, &result, &newEffect]
	{
		if (result)
		{
			EAE6320_ASSERT(newEffect != nullptr);
			o_effect = newEffect;
		}
		else
		{
			if (newEffect)
			{
				newEffect->DecrementReferenceCount();
				newEffect = nullptr;
			}
			o_effect = nullptr;
		}
	});


	// Allocate a new shader
	{
		newEffect = new (std::nothrow) EffectAsset();
		if (!newEffect)
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF(false, "Couldn't allocate memory for the geometry %s", i_path.c_str());
			Logging::OutputError("Failed to allocate memory for the geometry %s", i_path.c_str());
			return result;
		}
	}



	// Initialize the platform-specific graphics API shader object
	if (!(result = newEffect->InitData(i_path)))
	{
		EAE6320_ASSERTF(false, "Initialization of new geometry failed");
		return result;
	}

	return result;
}


eae6320::cResult eae6320::Graphics::EffectAsset::InitData(std::string i_path)
{
	auto result = eae6320::Results::Success;
	std::string errorMessage;
	eae6320::Platform::sDataFromFile dataFromFile;

	auto resultReadBinaryFile = eae6320::Platform::LoadBinaryFile(i_path.c_str(), dataFromFile, &errorMessage);
	if (!resultReadBinaryFile)
	{
		result = resultReadBinaryFile;
		EAE6320_ASSERTF(false, "Couldn't read binary file at path %s", i_path.c_str());
		Logging::OutputError("Couldn't read binary file at path %s", i_path.c_str());
		return result;
	}

	auto currentOffset = reinterpret_cast<uintptr_t>(dataFromFile.data);

	m_vertexShaderPath = reinterpret_cast<const char*>(currentOffset);

	currentOffset += m_vertexShaderPath.size() + 1;

	m_fragmentShaderPath = reinterpret_cast<const char*>(currentOffset);
	
	return result;
}