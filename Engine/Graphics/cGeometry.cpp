#include "cGeometry.h"
#include "PlatformIncludes.h"
#include "GraphicsEnv.h"
#include "sContext.h"
#include <Engine/Asserts/Asserts.h>

namespace eae6320
{
	namespace Graphics
	{
		namespace Geometry
		{
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

			cResult cGeometryRenderTarget::Factory(cGeometryRenderTarget*& o_geometryRenderTarget)
			{
				auto result = Results::Success;
				o_geometryRenderTarget = new cGeometryRenderTarget();
				o_geometryRenderTarget->m_referenceCount = 1;
				o_geometryRenderTarget->m_isInitialized = false;
#if defined( EAE6320_PLATFORM_D3D )
				o_geometryRenderTarget->m_vertexBuffer = nullptr;
				o_geometryRenderTarget->m_indexBuffer = nullptr;
#elif defined( EAE6320_PLATFORM_GL )
				o_geometryRenderTarget->m_vertexBufferId = 0;
				o_geometryRenderTarget->m_indexBufferId = 0;
				o_geometryRenderTarget->m_vertexArrayId = 0;
#endif
				return result;
			}

			void cGeometryRenderTarget::SetToPointer(cGeometryRenderTarget* &i_geometryRenderTarget)
			{
				if (this != i_geometryRenderTarget)
				{
					if (i_geometryRenderTarget != nullptr)
					{
						i_geometryRenderTarget->DecrementReferenceCount();
					}
					this->IncrementReferenceCount();
					i_geometryRenderTarget = this;
				}
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

			void cGeometryRenderTarget::AddFace(const cGeometryIndexFace &face)
			{
				std::vector<unsigned int> result = face.GetGeometryIndices();
				for (int i = 0; i < 3; i++) 
				{
					m_indices.push_back(result[i]);
				}
				UpdateData();
			}
			void cGeometryRenderTarget::AddIndices(int FaceNum, const std::vector<unsigned int> &triangleIndices)

			{
				for (int i = 0; i < FaceNum; i++)
				{
					AddFace(cGeometryIndexFace(triangleIndices[i * 3], triangleIndices[i * 3 + 1], triangleIndices[i * 3 + 2]));
				}
				UpdateData();
			}

			void cGeometryRenderTarget::AddVetices(int vertexNum, const std::vector<cGeometryVertex> &vertices)
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

			unsigned int* cGeometryRenderTarget::GetIndexData()
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

			unsigned int cGeometryRenderTarget::GetIndexCount()
			{
				return (unsigned int)m_indices.size();
			}		
		}
	}
}