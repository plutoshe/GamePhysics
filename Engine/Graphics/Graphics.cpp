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
				auto result_initGeometry = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_geometry->InitDevicePipeline();
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
					eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_effect->Bind();
					eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_geometry->Draw();
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