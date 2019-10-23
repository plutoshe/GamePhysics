#include "cGeometry.h"
#include "PlatformIncludes.h"
#include "GraphicsEnv.h"
#include "sContext.h"
#include <Engine/Asserts/Asserts.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>

namespace eae6320
{
	namespace Graphics
	{
		namespace Geometry
		{
			eae6320::Assets::cManager<eae6320::Graphics::Geometry::cGeometryRenderTarget> eae6320::Graphics::Geometry::cGeometryRenderTarget::s_manager;

			std::map<std::string, cGeometryRenderTarget::Handle> eae6320::Graphics::Geometry::cGeometryRenderTarget::s_hanlderMap = std::map<std::string, cGeometryRenderTarget::Handle>();
			void cGeometryRenderTarget::UpdateData()
			{
				m_isUpdateData = true;
			}
			void cGeometryRenderTarget::SetIndicesByFaces(std::vector<cGeometryIndexFace>& i_faces)
			{
				m_indices.clear();
				for (size_t faceId = 0; faceId <= i_faces.size(); faceId++)
				{
					std::vector<unsigned int> result = i_faces[faceId].GetGeometryIndices();
					for (int i = 0; i < 3; i++)
					{
						m_indices.push_back(result[i]);
					}
				}
				UpdateData();
			}

			void cGeometryRenderTarget::SetIndices(std::vector<unsigned int>& i_indices)
			{
				m_indices.clear();
				for (size_t i = 0; i < i_indices.size() / 3; i++)
				{
					AddFace(cGeometryIndexFace(i_indices[i * 3], i_indices[i * 3 + 1], i_indices[i * 3 + 2]));
				}
				UpdateData();
			}

			void cGeometryRenderTarget::SetVertices(std::vector<cGeometryVertex>& i_vertices)
			{
				m_vertices = i_vertices;
				UpdateData();
			}


#if defined( EAE6320_PLATFORM_GL )
			std::vector<unsigned int>  cGeometryIndexFace::GetGeometryIndices() const
			{
				return std::vector<unsigned int>{ m_i0, m_i1, m_i2 };
			}
#elif defined( EAE6320_PLATFORM_D3D )
			std::vector<unsigned int> cGeometryIndexFace::GetGeometryIndices() const
			{
				return std::vector<unsigned int>{ m_i0, m_i2, m_i1 };
			}
#endif

			void cGeometryRenderTarget::AddFace(const cGeometryIndexFace& face)
			{
				std::vector<unsigned int> result = face.GetGeometryIndices();
				for (int i = 0; i < 3; i++)
				{
					m_indices.push_back(result[i]);
				}
				UpdateData();
			}
			void cGeometryRenderTarget::AddIndices(int FaceNum, const std::vector<unsigned int>& triangleIndices)

			{
				for (int i = 0; i < FaceNum; i++)
				{
					AddFace(cGeometryIndexFace(triangleIndices[i * 3], triangleIndices[i * 3 + 1], triangleIndices[i * 3 + 2]));
				}
				UpdateData();
			}

			void cGeometryRenderTarget::AddVetices(int vertexNum, const std::vector<cGeometryVertex>& vertices)
			{
				m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.begin() + vertexNum);
				UpdateData();
			}

			unsigned int cGeometryRenderTarget::VertexBufferSize()
			{
				return static_cast<unsigned int>(m_vertices.size() * sizeof(cGeometryVertex));
			}

			unsigned int cGeometryRenderTarget::IndexBufferSize()
			{
				return static_cast<unsigned int>(m_indices.size() * sizeof(unsigned int));
			}

			cGeometryVertex* cGeometryRenderTarget::GetVertexData()
			{
				return &m_vertices[0];
			}

			uint16_t* cGeometryRenderTarget::GetIndexData()
			{
				return &m_indices[0];
			}

			unsigned int cGeometryRenderTarget::vertexCountToRender()
			{
				return static_cast<unsigned int>(m_vertices.size());
			}

			void cGeometryRenderTarget::InitData(const std::vector<cGeometryVertex>& vertices, const std::vector<unsigned int>& triangleIndices)
			{
				AddVetices((int)vertices.size(), vertices);
				AddIndices((int)triangleIndices.size() / 3, triangleIndices);
				UpdateData();
			}


			eae6320::cResult cGeometryRenderTarget::InitData(std::string i_path)
			{
				auto result = eae6320::Results::Success;
				std::string errorMessage;
				eae6320::Platform::sDataFromFile dataFromFile;
				
				auto resultReadBinaryFile = eae6320::Platform::LoadBinaryFile(i_path.c_str(), dataFromFile, &errorMessage);
				if (!resultReadBinaryFile)
				{
					result = resultReadBinaryFile;
					EAE6320_ASSERTF(false, "Couldn't read binary file at path %s", i_path.c_str());
					Logging::OutputError("Couldn't read binary file at path %s", i_path.c_str());
					return result;
				}

				auto currentOffset = reinterpret_cast<uintptr_t>(dataFromFile.data);
				uint16_t vertexCount, indexCount;
				size_t sizeS = 0;
				if (sizeS + sizeof(uint16_t) > dataFromFile.size)
				{
					EAE6320_ASSERTF(false, "Wrong file size at path %s", i_path.c_str());
					Logging::OutputError("Wrong file size at path %s", i_path.c_str());
					return result;
				}

				memcpy(&vertexCount, reinterpret_cast<void*>(currentOffset), sizeof(uint16_t));
				currentOffset += sizeof(uint16_t);
				sizeS += sizeof(uint16_t);
				
				if (sizeS + sizeof(uint16_t) > dataFromFile.size)
				{
					EAE6320_ASSERTF(false, "Wrong file size at path %s", i_path.c_str());
					Logging::OutputError("Wrong file size at path %s", i_path.c_str());
					return result;
				}

				memcpy(&indexCount, reinterpret_cast<void*>(currentOffset), sizeof(uint16_t));
				currentOffset += sizeof(uint16_t);
				sizeS += sizeof(uint16_t);

				if (sizeS + indexCount * sizeof(unsigned int) > dataFromFile.size)
				{
					EAE6320_ASSERTF(false, "Wrong file size at path %s", i_path.c_str());
					Logging::OutputError("Wrong file size at path %s", i_path.c_str());
					return result;
				}

				m_indices.resize(indexCount);
				memcpy(&m_indices[0], reinterpret_cast<void*>(currentOffset), indexCount * sizeof(uint16_t));
				currentOffset += indexCount * sizeof(uint16_t);
				sizeS += indexCount * sizeof(uint16_t);

				if (sizeS + vertexCount * sizeof(cGeometryVertex) > dataFromFile.size)
				{
					EAE6320_ASSERTF(false, "Wrong file size at path %s", i_path.c_str());
					Logging::OutputError("Wrong file size at path %s", i_path.c_str());
					return result;
				}

				m_vertices.resize(vertexCount);
				memcpy(&m_vertices[0], reinterpret_cast<void*>(currentOffset), vertexCount * sizeof(cGeometryVertex));
				

				UpdateData();
				return result;
			}

			uint16_t cGeometryRenderTarget::GetIndexCount()
			{
				return (uint16_t)m_indices.size();
			}

			
			eae6320::cResult cGeometry::Load()
			{
				auto result = eae6320::Results::Success;
				if (m_path == "")
				{
					EAE6320_ASSERTF(false, "Load Geometry failed");
					return  eae6320::Results::Failure;
				}
				if (cGeometryRenderTarget::s_hanlderMap.find(m_path) != cGeometryRenderTarget::s_hanlderMap.end())
				{
					m_handler = cGeometryRenderTarget::s_hanlderMap[m_path];
				}
				else if (!(result = cGeometryRenderTarget::s_manager.Load(
					m_path,
					m_handler)))
				{
					EAE6320_ASSERTF(false, "Load Geometry failed");
					return result;
				}
				else {
					cGeometryRenderTarget::s_hanlderMap[m_path] = m_handler;
				}
				
				return result;
			}
			eae6320::cResult cGeometryRenderTarget::Load(const std::string& i_path, cGeometryRenderTarget*& o_geometry)
			{
				auto result = Results::Success;

				cGeometryRenderTarget* newGeometry = nullptr;
				cScopeGuard scopeGuard([&o_geometry, &result, &newGeometry]
					{
						if (result)
						{
							EAE6320_ASSERT(newGeometry != nullptr);
							o_geometry = newGeometry;
						}
						else
						{
							if (newGeometry)
							{
								newGeometry->DecrementReferenceCount();
								newGeometry = nullptr;
							}
							o_geometry = nullptr;
						}
					});


				// Allocate a new shader
				{
					newGeometry = new (std::nothrow) cGeometryRenderTarget();
					if (!newGeometry)
					{
						result = Results::OutOfMemory;
						EAE6320_ASSERTF(false, "Couldn't allocate memory for the geometry %s", i_path.c_str());
						Logging::OutputError("Failed to allocate memory for the geometry %s", i_path.c_str());
						return result;
					}
				}



				// Initialize the platform-specific graphics API shader object
				if (!(result = newGeometry->InitData(i_path)))
				{
					EAE6320_ASSERTF(false, "Initialization of new geometry failed");
					return result;
				}

				return result;
			}
		}

	}

}


