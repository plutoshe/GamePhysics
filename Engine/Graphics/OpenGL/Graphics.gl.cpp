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

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>
#include <utility>

// Helper Function Declarations
//=============================

namespace
{
	eae6320::cResult InitializeGeometry();
	eae6320::cResult InitializeShadingData();
}

// Interface
//==========

// Submission
//-----------

void eae6320::Graphics::SubmitElapsedTime( const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime )
{
	EAE6320_ASSERT(eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread );
	auto& constantData_frame = eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread->constantData_frame;
	constantData_frame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
	constantData_frame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
}

eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted( const unsigned int i_timeToWait_inMilliseconds )
{
	return Concurrency::WaitForEvent(eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds );
}

eae6320::cResult eae6320::Graphics::SignalThatAllDataForAFrameHasBeenSubmitted()
{
	return eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
}

// Render
//-------

void eae6320::Graphics::RenderFrame()
{
	// Wait for the application loop to submit data to be rendered
	{
		const auto result = Concurrency::WaitForEvent(eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread );
		if ( result )
		{
			// Switch the render data pointers so that
			// the data that the application just submitted becomes the data that will now be rendered
			std::swap(eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread, eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread );
			// Once the pointers have been swapped the application loop can submit new data
			const auto result = eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal();
			if ( !result )
			{
				EAE6320_ASSERTF( false, "Couldn't signal that new graphics data can be submitted" );
				Logging::OutputError( "Failed to signal that new render data can be submitted" );
				UserOutput::Print( "The renderer failed to signal to the application that new graphics data can be submitted."
					" The application is probably in a bad state and should be exited" );
				return;
			}
		}
		else
		{
			EAE6320_ASSERTF( false, "Waiting for the graphics data to be submitted failed" );
			Logging::OutputError( "Waiting for the application loop to submit data to be rendered failed" );
			UserOutput::Print( "The renderer failed to wait for the application to submit data to be rendered."
				" The application is probably in a bad state and should be exited" );
			return;
		}
	}

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		// Black is usually used
		{
			glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
		{
			constexpr GLbitfield clearColor = GL_COLOR_BUFFER_BIT;
			glClear( clearColor );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
	}
	// In addition to the color buffer there is also a hidden image called the "depth buffer"
	// which is used to make it less important which order draw calls are made.
	// It must also be "cleared" every frame just like the visible color buffer.
	{
		{
			glDepthMask( GL_TRUE );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
			constexpr GLclampd clearToFarDepth = 1.0;
			glClearDepth( clearToFarDepth );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
		{
			constexpr GLbitfield clearDepth = GL_DEPTH_BUFFER_BIT;
			glClear( clearDepth );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
	}

	EAE6320_ASSERT(eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread );

	// Update the frame constant buffer
	{
		// Copy the data from the system memory that the application owns to GPU memory
		auto& constantData_frame = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->constantData_frame;
		eae6320::Graphics::Env::s_constantBuffer_frame.Update( &constantData_frame );
	}

	// Bind the shading data
	{
		eae6320::Graphics::Env::s_effect.Bind(cShader::s_manager, eae6320::Graphics::Env::s_vertexShader, eae6320::Graphics::Env::s_fragmentShader);
	}
	// Draw the geometry
	{
		for (size_t i = 0; i < eae6320::Graphics::Env::s_geometries.size(); i++)
		{
			eae6320::Graphics::Env::s_geometries[i].Draw();
		}
	}

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

eae6320::cResult eae6320::Graphics::CleanUp()
{
	auto result = Results::Success;
	for (size_t i = 0; i < eae6320::Graphics::Env::s_geometries.size(); i++)
	{
		const auto result_geometry = eae6320::Graphics::Env::s_geometries[i].Release();
		if (!result_geometry)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_geometry;
			}
		}
	}

	if (eae6320::Graphics::Env::s_programId != 0 )
	{
		glDeleteProgram(eae6320::Graphics::Env::s_programId );
		const auto errorCode = glGetError();
		if ( errorCode != GL_NO_ERROR )
		{
			if ( result )
			{
				result = eae6320::Results::Failure;
			}
			EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			eae6320::Logging::OutputError( "OpenGL failed to delete the program: %s",
				reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
		}
		eae6320::Graphics::Env::s_programId = 0;
	}
	if (eae6320::Graphics::Env::s_vertexShader )
	{
		const auto result_vertexShader = cShader::s_manager.Release(eae6320::Graphics::Env::s_vertexShader );
		if ( !result_vertexShader )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = result_vertexShader;
			}
		}
	}
	if (eae6320::Graphics::Env::s_fragmentShader )
	{
		const auto result_fragmentShader = cShader::s_manager.Release(eae6320::Graphics::Env::s_fragmentShader );
		if ( !result_fragmentShader )
		{
			EAE6320_ASSERT( false );
			if ( result )
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

		// Vertex Buffer

		std::vector<eae6320::Graphics::Geometry::cGeometryVertex> verticesA{
			eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 0.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(1.0f, 0.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 1.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(1.0f, 1.0f, 0.0f),
		};
		std::vector<unsigned int> indicesA{ 0, 1, 2, 1, 3, 2 };

		std::vector<eae6320::Graphics::Geometry::cGeometryVertex> verticesB{
			eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, -1.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(0.0f, -1.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, 0.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 0.0f, 0.0f),
		};

		eae6320::Graphics::Geometry::cGeometryRenderTarget geometryA, geometryB;
		geometryA.InitData(verticesA, indicesA);
		geometryB.InitData(verticesB, indicesA);
		
		eae6320::Graphics::Env::s_geometries.push_back(
			geometryA
		);
		eae6320::Graphics::Env::s_geometries.push_back(
			geometryB
		);
		for (size_t i = 0; i < eae6320::Graphics::Env::s_geometries.size(); i++)
		{
			auto result_initGeometry = eae6320::Graphics::Env::s_geometries[i].InitDevicePipeline();
			if (!result_initGeometry)
			{
				EAE6320_ASSERT(false);
				if (result)
				{
					result = result_initGeometry;
				}
			}

		}

		return result;

	}

	eae6320::cResult InitializeShadingData()
	{
		auto result = eae6320::Results::Success;
		eae6320::Graphics::Env::s_effect.SetVertexShaderPath("data/shaders/vertex/standard.shader");
		eae6320::Graphics::Env::s_effect.SetFragmentShaderPath("data/shaders/fragment/change_color.shader");
		if (!(result = eae6320::Graphics::Env::s_effect.Load(eae6320::Graphics::cShader::s_manager, eae6320::Graphics::Env::s_vertexShader, eae6320::Graphics::Env::s_fragmentShader)))
		{
			EAE6320_ASSERTF(false, "Can't initialize effects");
			return result;
		}
		{
			constexpr uint8_t defaultRenderState = 0;
			if (!(result = eae6320::Graphics::cRenderState::s_manager.Load(defaultRenderState, eae6320::Graphics::Env::s_renderState)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
				return result;
			}
		}

		// Create a program
		eae6320::cScopeGuard scopeGuard_program( [&result]
			{
				if ( !result )
				{
					if (eae6320::Graphics::Env::s_programId != 0 )
					{
						glDeleteProgram(eae6320::Graphics::Env::s_programId );
						const auto errorCode = glGetError();
						if ( errorCode != GL_NO_ERROR )
						{
							EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							eae6320::Logging::OutputError( "OpenGL failed to delete the program: %s",
								reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						}
						eae6320::Graphics::Env::s_programId = 0;
					}
				}
			} );
		{
			eae6320::Graphics::Env::s_programId = glCreateProgram();
			const auto errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to create a program: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				return result;
			}
			else if (eae6320::Graphics::Env::s_programId == 0 )
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERT( false );
				eae6320::Logging::OutputError( "OpenGL failed to create a program" );
				return result;
			}
		}
		// Attach the shaders to the program
		{
			// Vertex
			{
				glAttachShader(eae6320::Graphics::Env::s_programId, eae6320::Graphics::cShader::s_manager.Get(eae6320::Graphics::Env::s_vertexShader )->m_shaderId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to attach the vertex shader to the program: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					return result;
				}
			}
			// Fragment
			{
				glAttachShader(eae6320::Graphics::Env::s_programId, eae6320::Graphics::cShader::s_manager.Get(eae6320::Graphics::Env::s_fragmentShader )->m_shaderId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to attach the fragment shader to the program: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					return result;
				}
			}
		}
		// Link the program
		{
			glLinkProgram(eae6320::Graphics::Env::s_programId );
			const auto errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				// Get link info
				// (this won't be used unless linking fails
				// but it can be useful to look at when debugging)
				std::string linkInfo;
				{
					GLint infoSize;
					glGetProgramiv(eae6320::Graphics::Env::s_programId, GL_INFO_LOG_LENGTH, &infoSize );
					const auto errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						struct sLogInfo
						{
							GLchar* memory;
							sLogInfo( const size_t i_size ) { memory = reinterpret_cast<GLchar*>( malloc( i_size ) ); }
							~sLogInfo() { if ( memory ) free( memory ); }
						} info( static_cast<size_t>( infoSize ) );
						constexpr GLsizei* const dontReturnLength = nullptr;
						glGetProgramInfoLog(eae6320::Graphics::Env::s_programId, static_cast<GLsizei>( infoSize ), dontReturnLength, info.memory );
						const auto errorCode = glGetError();
						if ( errorCode == GL_NO_ERROR )
						{
							linkInfo = info.memory;
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							eae6320::Logging::OutputError( "OpenGL failed to get link info of the program: %s",
								reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						eae6320::Logging::OutputError( "OpenGL failed to get the length of the program link info: %s",
							reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						return result;
					}
				}
				// Check to see if there were link errors
				GLint didLinkingSucceed;
				{
					glGetProgramiv(eae6320::Graphics::Env::s_programId, GL_LINK_STATUS, &didLinkingSucceed );
					const auto errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						if ( didLinkingSucceed == GL_FALSE )
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF( false, linkInfo.c_str() );
							eae6320::Logging::OutputError( "The program failed to link: %s",
								linkInfo.c_str() );
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						eae6320::Logging::OutputError( "OpenGL failed to find out if linking of the program succeeded: %s",
							reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						return result;
					}
				}
			}
			else
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to link the program: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				return result;
			}
		}

		return result;
	}
}
