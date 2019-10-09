#include "Graphics.h"
#include "GraphicsEnv.h"
#include "cRenderObject.h"
#include <vector>
#include <Engine/UserOutput/UserOutput.h>

namespace eae6320
{
	namespace Graphics
	{
		void ChangeBackgroundColor(std::vector<float> updateColor)
		{
			eae6320::Graphics::Env::s_BackgroundColor = updateColor;
		}



		eae6320::cResult eae6320::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
		{
			auto result = Results::Success;

			// Initialize the platform-specific context
			if (!(result = sContext::g_context.Initialize(i_initializationParameters)))
			{
				EAE6320_ASSERTF(false, "Can't initialize Graphics without context");
				return result;
			}
			// Initialize the asset managers
			{
				if (!(result = cRenderState::s_manager.Initialize()))
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without the render state manager");
					return result;
				}
				if (!(result = cShader::s_manager.Initialize()))
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without the shader manager");
					return result;
				}
				if (!(result = Geometry::cGeometryRenderTarget::s_manager.Initialize()))
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without the render state manager");
					return result;
				}
			}

			// Initialize the platform-independent graphics objects
			{
				if (result = eae6320::Graphics::Env::s_constantBuffer_frame.Initialize())
				{
					// There is only a single frame constant buffer that is reused
					// and so it can be bound at initialization time and never unbound
					eae6320::Graphics::Env::s_constantBuffer_frame.Bind(
						// In our class both vertex and fragment shaders use per-frame constant data
						ShaderTypes::Vertex | ShaderTypes::Fragment);
				}
				else
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without frame constant buffer");
					return result;
				}

				if (result = eae6320::Graphics::Env::s_constantBuffer_drawCall.Initialize())
				{
					// There is only a single frame constant buffer that is reused
					// and so it can be bound at initialization time and never unbound
					eae6320::Graphics::Env::s_constantBuffer_drawCall.Bind(ShaderTypes::Vertex | ShaderTypes::Fragment);
				}
				else
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without drawCall constant buffer");
					return result;
				}
			}


			// Initialize the events
			{
				if (!(result = eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled)))
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data has been submitted from the application thread");
					return result;
				}
				if (!(result = eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
					Concurrency::EventState::Signaled)))
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data can be submitted from the application thread");
					return result;
				}
			}

			// Initialize the shading data
			{
				if (!(result = InitializeShadingData()))
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
					return result;
				}
			}


			// Initialize the geometry
			{
				if (!(result = InitializeGeometry()))
				{
					EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
					return result;
				}
			}

			{
				if (!(result = PlatformSpecificInitialization(i_initializationParameters)))
				{
					EAE6320_ASSERTF(false, "Can't initialize for platform specific setting");
					return result;
				}

			}
			
			return result;
		}



		eae6320::cResult InitializeGeometry()
		{
			return eae6320::Results::Success;

		}

		eae6320::cResult LoadShaderData(
			std::string path,
			std::map<std::string, eae6320::Graphics::cShader::Handle>& shaderMap,
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

		void SubmitElapsedTime(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime)
		{
			EAE6320_ASSERT(eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread);
			auto& constantData_frame = eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread->constantData_frame;
			constantData_frame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
			constantData_frame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
		}

		eae6320::cResult WaitUntilDataForANewFrameCanBeSubmitted(const unsigned int i_timeToWait_inMilliseconds)
		{
			return Concurrency::WaitForEvent(eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds);
		}

		eae6320::cResult SignalThatAllDataForAFrameHasBeenSubmitted()
		{
			return eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
		}



		void eae6320::Graphics::RenderFrame()
		{
			// Wait for the application loop to submit data to be rendered
			{
				const auto result = Concurrency::WaitForEvent(eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread);
				if (result)
				{
					// Switch the render data pointers so that
					// the data that the application just submitted becomes the data that will now be rendered
					std::swap(eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread, eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread);
					eae6320::Graphics::Env::s_waitingClearRenderObjects.clear();
					eae6320::Graphics::Env::s_waitingClearRenderObjects = eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread->m_renderObjects;
					// Once the pointers have been swapped the application loop can submit new data
					const auto result = eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal();
					if (!result)
					{
						EAE6320_ASSERTF(false, "Couldn't signal that new graphics data can be submitted");
						Logging::OutputError("Failed to signal that new render data can be submitted");
						UserOutput::Print("The renderer failed to signal to the application that new graphics data can be submitted."
							" The application is probably in a bad state and should be exited");
						return;
					}
				}
				else
				{
					EAE6320_ASSERTF(false, "Waiting for the graphics data to be submitted failed");
					Logging::OutputError("Waiting for the application loop to submit data to be rendered failed");
					UserOutput::Print("The renderer failed to wait for the application to submit data to be rendered."
						" The application is probably in a bad state and should be exited");
					return;
				}
			}
			EAE6320_ASSERT(eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread);

			// Update the frame constant buffer
			{
				// Copy the data from the system memory that the application owns to GPU memory
				auto& constantData_frame = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->constantData_frame;
				eae6320::Graphics::Env::s_constantBuffer_frame.Update(&constantData_frame);
				ChangeBackgroundColor(eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_backgroundColor);
				  
			}
			
			PrepocessBeforeRender();

			for (size_t i = 0; i < eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects.size(); i++)
			{
				auto result_initGeometry = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_geometry.InitDevicePipeline();
				if (!result_initGeometry)
				{
					EAE6320_ASSERT(false);
				}
			}
			auto result = eae6320::Results::Success;
			for (size_t i = 0; i < eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects.size(); i++)
			{
				if (!(result = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_effect->Load(
					eae6320::Graphics::cShader::s_manager)))
				{
					EAE6320_ASSERTF(false, "Can't initialize effects");
				}

			}

			// Bind the shading data
			// Draw the geometry
			{
				for (size_t i = 0; i < eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects.size(); i++)
				{
					auto& constantData_drawCall = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_Transformation;
					eae6320::Graphics::Env::s_constantBuffer_drawCall.Update(&constantData_drawCall);
					eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_effect->Bind();
					eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_geometry.Draw();
				}
			}
			PostpocessAfterRender();
		}

		eae6320::cResult eae6320::Graphics::CleanUp()
		{
			auto result = Results::Success;
			if (!(result = PlatformCleanUp()))
			{
				EAE6320_ASSERT(false);
			}
			eae6320::Graphics::Env::s_waitingClearRenderObjects.clear();
			eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects.clear();
			eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread->m_renderObjects.clear();
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
			for (auto it = eae6320::Graphics::Geometry::cGeometryRenderTarget::s_hanlderMap.begin(); it != eae6320::Graphics::Geometry::cGeometryRenderTarget::s_hanlderMap.end(); ++it)
			{
				const auto result_geometry = eae6320::Graphics::Geometry::cGeometryRenderTarget::s_manager.Release(it->second);
				if (!result_geometry)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_geometry;
					}
				}
			}


			if (eae6320::Graphics::Env::s_renderState)
			{
				const auto result_renderState = cRenderState::s_manager.Release(eae6320::Graphics::Env::s_renderState);
				if (!result_renderState)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_renderState;
					}
				}
			}
			{
				const auto result_constantBuffer_drawCall = eae6320::Graphics::Env::s_constantBuffer_drawCall.CleanUp();
				if (!result_constantBuffer_drawCall)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_constantBuffer_drawCall;
					}
				}
			}


			{
				const auto result_constantBuffer_frame = eae6320::Graphics::Env::s_constantBuffer_frame.CleanUp();
				if (!result_constantBuffer_frame)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_constantBuffer_frame;
					}
				}
			}
			{
				const auto result_geometryManager = Geometry::cGeometryRenderTarget::s_manager.CleanUp();
				if (!result_geometryManager)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_geometryManager;
					}
				}
			}
			{
				const auto result_shaderManager = cShader::s_manager.CleanUp();
				if (!result_shaderManager)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_shaderManager;
					}
				}
			}
			{
				const auto result_renderStateManager = cRenderState::s_manager.CleanUp();
				if (!result_renderStateManager)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_renderStateManager;
					}
				}
			}

			{
				const auto result_context = sContext::g_context.CleanUp();
				if (!result_context)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_context;
					}
				}
			}

			return result;
		}

	}

}