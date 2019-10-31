#include "../cEffect.h"
#include "../PlatformIncludes.h"
#include "../Graphics.h"
#include "../GraphicsEnv.h"

#include <Engine/ScopeGuard/cScopeGuard.h>

eae6320::cResult eae6320::Graphics::EffectAsset::Release()
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


eae6320::cResult eae6320::Graphics::cEffect::Prepare(
	eae6320::Assets::cManager<eae6320::Graphics::cShader>& manager)
{
	EffectAsset* effect = EffectAsset::s_manager.Get(m_handler);
	auto result = eae6320::Results::Success;
	if (!effect->m_isInitialized)
	{
		effect->m_isInitialized = true;
		if (!(result = PrepareShaderData()))
		{
			EAE6320_ASSERTF(false, "Can't initialize shader for effect");
			return result;
		}
		eae6320::cScopeGuard scopeGuard_program([&effect, &result]
			{
				if (!result)
				{
					if (effect->m_programId != 0)
					{
						glDeleteProgram(effect->m_programId);
						const auto errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
						}
						effect->m_programId = 0;
					}
				}
			});

		// Create a program

		{
			effect->m_programId = glCreateProgram();
			const auto errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to create a program: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				return result;
			}
			else if (effect->m_programId == 0)
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
				glAttachShader(effect->m_programId, manager.Get(eae6320::Graphics::Env::s_vertexShaders[effect->m_vertexShaderPath])->m_shaderId);
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
				glAttachShader(effect->m_programId, manager.Get(eae6320::Graphics::Env::s_fragmentShaders[effect->m_fragmentShaderPath])->m_shaderId);
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
			glLinkProgram(effect->m_programId);
			const auto errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				// Get link info
				// (this won't be used unless linking fails
				// but it can be useful to look at when debugging)
				std::string linkInfo;
				{
					GLint infoSize;
					glGetProgramiv(effect->m_programId, GL_INFO_LOG_LENGTH, &infoSize);
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
						glGetProgramInfoLog(effect->m_programId, static_cast<GLsizei>(infoSize), dontReturnLength, info.memory);
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
					glGetProgramiv(effect->m_programId, GL_LINK_STATUS, &didLinkingSucceed);
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
	}
	return result;
}
