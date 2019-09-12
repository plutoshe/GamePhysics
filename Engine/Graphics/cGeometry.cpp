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
			}
			void cGeometryRenderTarget::AddIndices(int FaceNum, const std::vector<unsigned int> &triangleIndices)
			{
				for (int i = 0; i < FaceNum; i++)
				{
					AddFace(cGeometryIndexFace(triangleIndices[i * 3], triangleIndices[i * 3 + 1], triangleIndices[i * 3 + 2]));
				}
			}

			void cGeometryRenderTarget::AddVetices(int vertexNum, const std::vector<cGeometryVertex> &vertices)
			{
				m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.begin() + vertexNum);
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
			}

			unsigned int cGeometryRenderTarget::GetIndexCount()
			{
				return (unsigned int)m_indices.size();
			}		
		}
	}
}