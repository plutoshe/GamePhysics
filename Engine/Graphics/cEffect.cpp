#include "cEffect.h"

void eae6320::Graphics::Effect::AddShader(eae6320::Graphics::ShaderEffect shader)
{
	m_shaders.push_back(shader);
}
//template <class T>
eae6320::cResult eae6320::Graphics::Effect::Load(eae6320::Assets::cManager<eae6320::Graphics::cShader> &manager, eae6320::Graphics::cShader::Handle &vertexShader, eae6320::Graphics::cShader::Handle &fragmentShader)
{
	auto result = eae6320::Results::Success;

	for (size_t i = 0; i < m_shaders.size(); i++)
	{
		eae6320::Graphics::cShader::Handle *currentHandler = nullptr;
		switch (m_shaders[i].m_shaderType)
		{
		case eae6320::Graphics::ShaderTypes::Vertex:
			currentHandler = &vertexShader;
			break;
		case eae6320::Graphics::ShaderTypes::Fragment:
			currentHandler = &fragmentShader;
			break;
		}
		if (currentHandler == nullptr)
		{
			EAE6320_ASSERTF(false, "Load Shader failed");
			return eae6320::Results::Failure;
		}
		if (currentHandler != nullptr && !(result = manager.Load(m_shaders[i].m_shaderPath, *currentHandler, m_shaders[i].m_shaderType))) {
			EAE6320_ASSERTF(false, "Load Shader failed");
			return result;
		}
	}
	return result;
}