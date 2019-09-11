#include "Graphics.h"
#include "GraphicsEnv.h"
#include <vector>

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
				eae6320::Graphics::Geometry::cGeometryVertex(-0.3f, -0.3f, 0.0f),
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
			eae6320::Graphics::Effect effectA, effectB;
			effectA.SetVertexShaderPath("data/shaders/vertex/standard.shader");
			effectA.SetFragmentShaderPath("data/shaders/fragment/change_color.shader");
			effectB.SetVertexShaderPath("data/shaders/vertex/standard.shader");
			effectB.SetFragmentShaderPath("data/shaders/fragment/standard.shader");

			eae6320::Graphics::Env::s_effects.push_back(effectA);
			eae6320::Graphics::Env::s_effects.push_back(effectB);

			for (size_t i = 0; i < eae6320::Graphics::Env::s_effects.size(); i++)
			{
				if (!(result = LoadShaderData(
					eae6320::Graphics::Env::s_effects[i].m_vertexShaderPath,
					eae6320::Graphics::Env::s_vertexShaders,
					eae6320::Graphics::ShaderTypes::Vertex)) ||
					!(result = LoadShaderData(
						eae6320::Graphics::Env::s_effects[i].m_fragmentShaderPath,
						eae6320::Graphics::Env::s_fragmentShaders,
						eae6320::Graphics::ShaderTypes::Fragment)))
				{
					EAE6320_ASSERTF(false, "Can't initialize effects");
					return result;
				}
				if (!(result = eae6320::Graphics::Env::s_effects[i].Load(
					eae6320::Graphics::cShader::s_manager,
					eae6320::Graphics::Env::s_vertexShaders[eae6320::Graphics::Env::s_effects[i].m_vertexShaderPath],
					eae6320::Graphics::Env::s_fragmentShaders[eae6320::Graphics::Env::s_effects[i].m_fragmentShaderPath])))
				{
					EAE6320_ASSERTF(false, "Can't initialize effects");
					return result;
				}

			}

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
	}

}