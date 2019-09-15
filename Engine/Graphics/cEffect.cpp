#include "GraphicsEnv.h"
#include "cEffect.h"
#include "PlatformIncludes.h"

#include <Engine/Asserts/Asserts.h>

eae6320::cResult eae6320::Graphics::Effect::Bind()
{
	auto result = eae6320::Results::Success;
#if defined( EAE6320_PLATFORM_D3D )
	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);
	constexpr ID3D11ClassInstance* const* noInterfaces = nullptr;
	constexpr unsigned int interfaceCount = 0;
	// Vertex shader
	{
		if (eae6320::Graphics::Env::s_vertexShaders.find(m_vertexShaderPath) == eae6320::Graphics::Env::s_vertexShaders.end())
		{
			EAE6320_ASSERTF(false, "Can't load vertex shader");
			return eae6320::Results::Failure;
		}
		else {
			EAE6320_ASSERT(eae6320::Graphics::Env::s_vertexShaders[m_vertexShaderPath]);
			auto* const shader = cShader::s_manager.Get(eae6320::Graphics::Env::s_vertexShaders[m_vertexShaderPath]);
			EAE6320_ASSERT(shader && shader->m_shaderObject.vertex);
			direct3dImmediateContext->VSSetShader(shader->m_shaderObject.vertex, noInterfaces, interfaceCount);
		}
	}
	// Fragment shader
	{
		if (eae6320::Graphics::Env::s_fragmentShaders.find(m_fragmentShaderPath) == eae6320::Graphics::Env::s_fragmentShaders.end())
		{
			EAE6320_ASSERTF(false, "Can't load vertex shader");
			return eae6320::Results::Failure;
		}
		else {
			EAE6320_ASSERT(eae6320::Graphics::Env::s_fragmentShaders[m_fragmentShaderPath]);
			auto* const shader = cShader::s_manager.Get(eae6320::Graphics::Env::s_fragmentShaders[m_fragmentShaderPath]);
			EAE6320_ASSERT(shader && shader->m_shaderObject.fragment);
			direct3dImmediateContext->PSSetShader(shader->m_shaderObject.fragment, noInterfaces, interfaceCount);
		}
	}

#elif defined( EAE6320_PLATFORM_GL )

	EAE6320_ASSERT(m_programId != 0);
	glUseProgram(m_programId);
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

eae6320::cResult eae6320::Graphics::Effect::LoadShaderData()
{
	auto result = eae6320::Results::Success;
	if (!(result = eae6320::Graphics::LoadShaderData(
		m_vertexShaderPath,
		eae6320::Graphics::Env::s_vertexShaders,
		eae6320::Graphics::ShaderTypes::Vertex)) ||
		!(result = eae6320::Graphics::LoadShaderData(
			m_fragmentShaderPath,
			eae6320::Graphics::Env::s_fragmentShaders,
			eae6320::Graphics::ShaderTypes::Fragment)))
	{
		EAE6320_ASSERTF(false, "Can't initialize shader for effect");
	}
	return result;
}