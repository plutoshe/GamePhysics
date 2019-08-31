#include "cGeometry.h"
namespace eae6320
{
	namespace Graphics
	{
		namespace Geometry
		{
			#if defined( EAE6320_PLATFORM_GL )

				cGeometryVertex* cGeometryFace::GetGeometryVertices()
				{
					return new cGeometryVertex[3]{ m_v0, m_v1, m_v2 };
				}
			#elif defined( EAE6320_PLATFORM_D3D )
				cGeometryVertex* cGeometryFace::GetGeometryVertices()
				{
					return new cGeometryVertex[3]{ m_v0, m_v2, m_v1 };
				}
			#endif

			void cGeometryRenderTarget::AddFace(cGeometryFace face)
			{
				cGeometryVertex* result = face.GetGeometryVertices();
				for (int i = 0; i < 3; i++) 
				{
					vertices.push_back(result[i]);
				}
			}

			unsigned int cGeometryRenderTarget::BufferSize()
			{
				return static_cast<unsigned int>(vertices.size() * sizeof(cGeometryVertex));
			}

			cGeometryVertex* cGeometryRenderTarget::GetVertexData()
			{
				return &vertices[0];
			}
		}
	}
}