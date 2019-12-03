#pragma once

#include <Engine/Results/Results.h>
#include <xaudio2.h>
#include "3DAudio.h"
#include "AudioSource.h"
#include "XAudio2/XAudio2Helpers.h"

//#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>

// Interface
//==========

namespace
{
	eae6320::cResult LoadAssetBinary(const char* const i_path, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage)
	{
		eae6320::Platform::sDataFromFile dataFromFile;

		// here are our data fields to populate:
		char* filepath;

		auto result = eae6320::Platform::LoadBinaryFile(i_path, dataFromFile, errMessage);
		if (!result)
		{
			//EAE6320_ASSERTF(false, errMessage.c_str());
			eae6320::Logging::OutputError("Failed to load geometry from file %s: %s", i_path, errMessage);
			return result;
		}
		auto currentOffset = reinterpret_cast<uintptr_t>(dataFromFile.data);
		const auto finalOffset = currentOffset + dataFromFile.size;

		// extract in the order: vertexCount, indexCount, vertices, indices

		//// getting the vertex count
		//memcpy(&vertexCount, reinterpret_cast<void*>(currentOffset), sizeof(vertexCount));

		//// shift over to get the index count
		//currentOffset += sizeof(vertexCount);
		//memcpy(&indexCount, reinterpret_cast<void*>(currentOffset), sizeof(indexCount));

		// shift over to get the vertices array
		//currentOffset += sizeof(indexCount);

		// turns out we need to copy the memory because of the way the code is laid out since the original binary file data goes out of scope outside this function
		//eae6320::Logging::OutputMessage("Here's our filename %s", currentOffset);

		filepath = (char*) malloc(dataFromFile.size * sizeof(char));
		if (!filepath)
			return eae6320::Results::Failure;
		//vertices = reinterpret_cast<eae6320::Graphics::VertexFormats::s3dObject*>(currentOffset);
		memcpy(filepath, reinterpret_cast<void*>(currentOffset), dataFromFile.size * sizeof(filepath));

		//// now shift by the number of vertices to get the index array
		//currentOffset += vertexCount * sizeof(eae6320::Graphics::VertexFormats::s3dObject);
		////indices = reinterpret_cast<uint16_t*>(currentOffset);

		//indices = (uint16_t*)malloc(indexCount * sizeof(uint16_t));
		//if (!indices)
		//	return eae6320::Results::Failure;
		////vertices = reinterpret_cast<eae6320::Graphics::VertexFormats::s3dObject*>(currentOffset);
		//memcpy(indices, reinterpret_cast<void*>(currentOffset), indexCount * sizeof(uint16_t));


		//currentOffset += indexCount * sizeof(uint16_t);
		//EAE6320_ASSERT(currentOffset == finalOffset);

		// yayyy now we have all our data!
		/*o_initParams.m_nVertices = vertexCount;
		o_initParams.m_nIndices = indexCount;
		o_initParams.m_vertexData = vertices;
		o_initParams.m_indexData = indices;
*/

		o_initParams.filename = filepath;
		return result;
	}
}

namespace eae6320
{
	namespace Audio3D
	{
		cResult eae6320::Audio3D::AudioSource::Load(const char* i_path, AudioSource*& o_source)
		{
			auto result = Results::Success;

			//Platform::sDataFromFile dataFromFile;
			AudioSource* newGeom = nullptr;
			//AudioInitParams initParams;

			//// Load the now-binary data
			//{
			//	std::string errorMessage;
			//	if (!(result = LoadAssetBinary(i_path, initParams, &errorMessage)))
			//		{
			//		//EAE6320_ASSERTF(false, errorMessage.c_str());
			//		Logging::OutputError("Failed to load audio from file %s: %s", i_path, errorMessage.c_str());
			//		return result;
			//	}
			//	//eae6320::Logging::OutputMessage("Got vertex count as %d", initParams.m_nVertices);
			//}

			newGeom = new AudioSource();
			result = newGeom->Initialize(i_path);
			if (!result)
			{
				Logging::OutputError("Failed to initialize audio source object from file %s", i_path);
				return result;
			}
			o_source = newGeom;

			/*free(initParams.m_indexData);
			free(initParams.m_vertexData);*/

			return result;
		}
	}
}
