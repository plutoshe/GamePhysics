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
				if (!(result = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_effect.Load(
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
					eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_effect.Bind();
					eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->m_renderObjects[i].m_geometry.Draw();
				}
			}
			PostpocessAfterRender();
		}
	}

}