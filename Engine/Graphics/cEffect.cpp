#include "GraphicsEnv.h"
#include "cEffect.h"
#include "PlatformIncludes.h"

#include <Engine/Asserts/Asserts.h>



eae6320::cResult eae6320::Graphics::Effect::Load(eae6320::Assets::cManager<eae6320::Graphics::cShader> &manager, eae6320::Graphics::cShader::Handle &vertexShader, eae6320::Graphics::cShader::Handle &fragmentShader)
{
	auto result = eae6320::Results::Success;
	if (m_vertexShaderPath != "" && !(result = manager.Load(m_vertexShaderPath, vertexShader, eae6320::Graphics::ShaderTypes::Vertex))) {
		EAE6320_ASSERTF(false, "Load Shader failed");
		return result;
	}
	if (m_fragmentShaderPath != "" && !(result = manager.Load(m_fragmentShaderPath, fragmentShader, eae6320::Graphics::ShaderTypes::Fragment))) {
		EAE6320_ASSERTF(false, "Load Shader failed");
		return result;
	}
	
	return result;
}

void eae6320::Graphics::Effect::Bind(eae6320::Assets::cManager<eae6320::Graphics::cShader>& s_manager, eae6320::Graphics::cShader::Handle& s_vertexShader, eae6320::Graphics::cShader::Handle& s_fragmentShader)
{
#if defined( EAE6320_PLATFORM_D3D )
	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);
	constexpr ID3D11ClassInstance* const* noInterfaces = nullptr;
	constexpr unsigned int interfaceCount = 0;
	// Vertex shader
	{
		EAE6320_ASSERT(s_vertexShader);
		auto* const shader = cShader::s_manager.Get(s_vertexShader);
		EAE6320_ASSERT(shader && shader->m_shaderObject.vertex);
		direct3dImmediateContext->VSSetShader(shader->m_shaderObject.vertex, noInterfaces, interfaceCount);
	}
	// Fragment shader
	{
		EAE6320_ASSERT(s_fragmentShader);
		auto* const shader = cShader::s_manager.Get(s_fragmentShader);
		EAE6320_ASSERT(shader && shader->m_shaderObject.fragment);
		direct3dImmediateContext->PSSetShader(shader->m_shaderObject.fragment, noInterfaces, interfaceCount);
	}

#elif defined( EAE6320_PLATFORM_GL )
	EAE6320_ASSERT(eae6320::Graphics::Env::s_programId != 0);
	glUseProgram(eae6320::Graphics::Env::s_programId);
	EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
#endif
	// Render state
	{
		EAE6320_ASSERT(eae6320::Graphics::Env::s_renderState);
		auto* const renderState = cRenderState::s_manager.Get(eae6320::Graphics::Env::s_renderState);
		EAE6320_ASSERT(renderState);
		renderState->Bind();
	}
}

