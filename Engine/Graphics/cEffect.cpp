#include "GraphicsEnv.h"
#include "cEffect.h"
#include "PlatformIncludes.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/ScopeGuard/cScopeGuard.h>

std::map<std::string, eae6320::Graphics::cShader::Handle> eae6320::Graphics::Effect::vertexHandles;
std::map<std::string, eae6320::Graphics::cShader::Handle> eae6320::Graphics::Effect::fragmentHandles;
eae6320::cResult eae6320::Graphics::Effect::Release()
{
	auto result = eae6320::Results::Success;
	if (m_programId != 0)
	{
		glDeleteProgram(m_programId);
		const auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			if (result)
			{
				result = eae6320::Results::Failure;
			}
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		m_programId = 0;
	}
	return result;
}


eae6320::cResult eae6320::Graphics::Effect::Load(
	eae6320::Assets::cManager<eae6320::Graphics::cShader> &manager, 
	eae6320::Graphics::cShader::Handle &vertexShader, eae6320::Graphics::cShader::Handle &fragmentShader)
{
	auto result = eae6320::Results::Success;

	eae6320::cScopeGuard scopeGuard_program([this, &result]
		{
			if (!result)
			{
				if (this->m_programId != 0)
				{
					glDeleteProgram(this->m_programId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
					}
					this->m_programId = 0;
				}
			}
		});

	// Create a program

	{
		m_programId = glCreateProgram();
		const auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			result = eae6320::Results::Failure;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to create a program: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			return result;
		}
		else if (m_programId == 0)
		{
			result = eae6320::Results::Failure;
			EAE6320_ASSERT(false);
			eae6320::Logging::OutputError("OpenGL failed to create a program");
			return result;
		}
	}
	// Attach the shaders to the program
	{
		// Vertex
		{
			glAttachShader(m_programId, manager.Get(vertexShader)->m_shaderId);
			const auto errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to attach the vertex shader to the program: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				return result;
			}
		}
		// Fragment
		{
			glAttachShader(m_programId, manager.Get(fragmentShader)->m_shaderId);
			const auto errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to attach the fragment shader to the program: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				return result;
			}
		}
	}
	// Link the program
	{
		glLinkProgram(m_programId);
		const auto errorCode = glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			// Get link info
			// (this won't be used unless linking fails
			// but it can be useful to look at when debugging)
			std::string linkInfo;
			{
				GLint infoSize;
				glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoSize);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					struct sLogInfo
					{
						GLchar* memory;
						sLogInfo(const size_t i_size) { memory = reinterpret_cast<GLchar*>(malloc(i_size)); }
						~sLogInfo() { if (memory) free(memory); }
					} info(static_cast<size_t>(infoSize));
					constexpr GLsizei* const dontReturnLength = nullptr;
					glGetProgramInfoLog(m_programId, static_cast<GLsizei>(infoSize), dontReturnLength, info.memory);
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						linkInfo = info.memory;
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to get link info of the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to get the length of the program link info: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Check to see if there were link errors
			GLint didLinkingSucceed;
			{
				glGetProgramiv(m_programId, GL_LINK_STATUS, &didLinkingSucceed);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					if (didLinkingSucceed == GL_FALSE)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, linkInfo.c_str());
						eae6320::Logging::OutputError("The program failed to link: %s",
							linkInfo.c_str());
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to find out if linking of the program succeeded: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
		}
		else
		{
			result = eae6320::Results::Failure;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to link the program: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			return result;
		}
	}
	return result;
}

void eae6320::Graphics::Effect::Bind()
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
}

