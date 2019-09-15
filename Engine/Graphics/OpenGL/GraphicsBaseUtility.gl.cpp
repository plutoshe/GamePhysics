// Includes
//=========

#include "../Graphics.h"

#include "Includes.h"
#include "../cConstantBuffer.h"
#include "../ConstantBufferFormats.h"
#include "../cRenderState.h"
#include "../cShader.h"
#include "../sContext.h"
#include "../cGeometry.h"
#include "../cEffect.h"
#include "../GraphicsEnv.h"
#include "../cShader.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>
#include <utility>

namespace {
	eae6320::cResult InitializeShadingData();
}

void eae6320::Graphics::ClearBackgroundColor()
{
	if (eae6320::Graphics::Env::s_BackgroundColor.size() >= 4)
	{
		glClearColor(
			eae6320::Graphics::Env::s_BackgroundColor[0],
			eae6320::Graphics::Env::s_BackgroundColor[1],
			eae6320::Graphics::Env::s_BackgroundColor[2],
			eae6320::Graphics::Env::s_BackgroundColor[3]);
	}
	EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
}

void eae6320::Graphics::PrepocessBeforeRender()
{

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		{
			eae6320::Graphics::ClearBackgroundColor();
		}
		{
			constexpr GLbitfield clearColor = GL_COLOR_BUFFER_BIT;
			glClear(clearColor);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
	}
	// In addition to the color buffer there is also a hidden image called the "depth buffer"
	// which is used to make it less important which order draw calls are made.
	// It must also be "cleared" every frame just like the visible color buffer.
	{
		{
			glDepthMask(GL_TRUE);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
			constexpr GLclampd clearToFarDepth = 1.0;
			glClearDepth(clearToFarDepth);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		{
			constexpr GLbitfield clearDepth = GL_DEPTH_BUFFER_BIT;
			glClear(clearDepth);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
	}
}

	
void eae6320::Graphics::PostpocessAfterRender()
{
	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it the contents of the back buffer must be "presented"
	// (or "swapped" with the "front buffer", which is the image that is actually being displayed)
	{
		const auto deviceContext = sContext::g_context.deviceContext;
		EAE6320_ASSERT( deviceContext != NULL );
		const auto glResult = SwapBuffers( deviceContext );
		EAE6320_ASSERT( glResult != FALSE );
	}

	// Once everything has been drawn the data that was submitted for this frame
	// should be cleaned up and cleared.
	// so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
	{
		// (At this point in the class there isn't anything that needs to be cleaned up)
	}
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::Initialize( const sInitializationParameters& i_initializationParameters )
{
	auto result = Results::Success;

	// Initialize the platform-specific context
	if ( !( result = sContext::g_context.Initialize( i_initializationParameters ) ) )
	{
		EAE6320_ASSERTF( false, "Can't initialize Graphics without context" );
		return result;
	}
	// Initialize the asset managers
	{
		if ( !( result = cRenderState::s_manager.Initialize() ) )
		{
			EAE6320_ASSERTF( false, "Can't initialize Graphics without the render state manager" );
			return result;
		}
		if ( !( result = cShader::s_manager.Initialize() ) )
		{
			EAE6320_ASSERTF( false, "Can't initialize Graphics without the shader manager" );
			return result;
		}
	}
	// Initialize the platform-independent graphics objects
	{
		if ( result = eae6320::Graphics::Env::s_constantBuffer_frame.Initialize() )
		{
			// There is only a single frame constant buffer that is reused
			// and so it can be bound at initialization time and never unbound
			eae6320::Graphics::Env::s_constantBuffer_frame.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				ShaderTypes::Vertex | ShaderTypes::Fragment );
		}
		else
		{
			EAE6320_ASSERTF( false, "Can't initialize Graphics without frame constant buffer" );
			return result;
		}
	}
	// Initialize the events
	{
		if ( !( result = eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize( Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled ) ) )
		{
			EAE6320_ASSERTF( false, "Can't initialize Graphics without event for when data has been submitted from the application thread" );
			return result;
		}
		if ( !( result = eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize( Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
			Concurrency::EventState::Signaled ) ) )
		{
			EAE6320_ASSERTF( false, "Can't initialize Graphics without event for when data can be submitted from the application thread" );
			return result;
		}
	}
	// Initialize the shading data
	{
		if ( !( result = InitializeShadingData() ) )
		{
			EAE6320_ASSERTF( false, "Can't initialize Graphics without the shading data" );
			return result;
		}
	}
	// Initialize the geometry
	{
		if ( !( result = InitializeGeometry() ) )
		{
			EAE6320_ASSERTF( false, "Can't initialize Graphics without the geometry data" );
			return result;
		}
	}

	return result;
}

eae6320::cResult eae6320::Graphics::InitializeShadingData()
{
	auto result = eae6320::Results::Success;
	{
		constexpr uint8_t defaultRenderState = 0;
		if (!(result = eae6320::Graphics::cRenderState::s_manager.Load(defaultRenderState, eae6320::Graphics::Env::s_renderState)))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
			return result;
		}
	}
	return result;
}

eae6320::cResult eae6320::Graphics::CleanUp()
{
	auto result = Results::Success;
	
	for (auto it = eae6320::Graphics::Env::s_vertexShaders.begin(); it != eae6320::Graphics::Env::s_vertexShaders.end(); ++it)
	{
		const auto result_vertexShader = cShader::s_manager.Release(it->second);
		if (!result_vertexShader)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_vertexShader;
			}
		}
	}
	for (auto it = eae6320::Graphics::Env::s_fragmentShaders.begin(); it != eae6320::Graphics::Env::s_fragmentShaders.end(); ++it)
	{
		const auto result_fragmentShader = cShader::s_manager.Release(it->second);
		if (!result_fragmentShader)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_fragmentShader;
			}
		}
	}
	
	if (eae6320::Graphics::Env::s_renderState )
	{
		const auto result_renderState = cRenderState::s_manager.Release(eae6320::Graphics::Env::s_renderState );
		if ( !result_renderState )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = result_renderState;
			}
		}
	}

	{
		const auto result_constantBuffer_frame = eae6320::Graphics::Env::s_constantBuffer_frame.CleanUp();
		if ( !result_constantBuffer_frame )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = result_constantBuffer_frame;
			}
		}
	}

	{
		const auto result_shaderManager = cShader::s_manager.CleanUp();
		if ( !result_shaderManager )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = result_shaderManager;
			}
		}
	}
	{
		const auto result_renderStateManager = cRenderState::s_manager.CleanUp();
		if ( !result_renderStateManager )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = result_renderStateManager;
			}
		}
	}
	
	{
		const auto result_context = sContext::g_context.CleanUp();
		if ( !result_context )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = result_context;
			}
		}
	}

	return result;
}

// Helper Function Definitions
//============================

namespace
{
	eae6320::cResult InitializeGeometry()
	{
		auto result = eae6320::Results::Success;
		return result;

	}

	eae6320::cResult LoadShaderData(
		std::string path, 
		std::map<std::string, eae6320::Graphics::cShader::Handle> &shaderMap,
		eae6320::Graphics::ShaderTypes::eType shaderType)
	{
		auto result = eae6320::Results::Success;
		if (path == "")
		{
			EAE6320_ASSERTF(false, "Load Shader failed");
			return  eae6320::Results::Failure;
		}
		if (shaderMap.find(path) == shaderMap.end())
		{
			shaderMap.insert(std::pair<std::string, eae6320::Graphics::cShader::Handle>(path, eae6320::Graphics::cShader::Handle()));
			if (!(result = eae6320::Graphics::cShader::s_manager.Load(
				path,
				shaderMap[path],
				shaderType)))
			{
				EAE6320_ASSERTF(false, "Load Shader failed");
				return result;
			}
		}
		return result;
	}

	eae6320::cResult InitializeShadingData()
	{
		auto result = eae6320::Results::Success;

		{
			constexpr uint8_t defaultRenderState = 0;
			if (!(result = eae6320::Graphics::cRenderState::s_manager.Load(defaultRenderState, eae6320::Graphics::Env::s_renderState)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
				return result;
			}
		}
		return result;
	}
}
