#pragma once

#include <vector>

#include "cVertexFormat.h"
#include "PlatformIncludes.h"

namespace eae6320
{
	namespace Graphics
	{
		namespace Geometry
		{
			class cGeometryVertex
			{
			public:
				float m_x, m_y, m_z;
				cGeometryVertex() { m_x = m_y = m_z = 0; }
				cGeometryVertex(float x, float y, float z) { m_x = x; m_y = y; m_z = z; }
				cGeometryVertex(const cGeometryVertex& c) { this->Equal(c); }
				void operator = (const cGeometryVertex& c) { this->Equal(c); }
				void Equal(const cGeometryVertex& c) { m_x = c.m_x; m_y = c.m_y; m_z = c.m_z; }
			};

			class cGeometryIndexFace
			{
			public:
				// default store as right-handed
				unsigned int m_i0, m_i1, m_i2;
				cGeometryIndexFace() { m_i0 = m_i1 = m_i2 = 0; }
				cGeometryIndexFace(const cGeometryIndexFace& f)
				{
					m_i0 = f.m_i0;
					m_i1 = f.m_i1;
					m_i2 = f.m_i2;
				}
				cGeometryIndexFace(unsigned int i0, unsigned int i1, unsigned int i2)
				{
					m_i0 = i0; m_i1 = i1; m_i2 = i2;
				}
				std::vector<unsigned int> GetGeometryIndices() const;
			};

			class cGeometryRenderTarget
			{
			public:
				
				unsigned int VertexBufferSize();
				unsigned int GetIndexCount();
				unsigned int IndexBufferSize();
				void AddFace(const cGeometryIndexFace &face);
				void AddIndices(int FaceNum, const std::vector<unsigned int> &triangleIndices);
				void AddVetices(int vertexNum, const std::vector<cGeometryVertex>& vertices);
				cGeometryVertex* GetVertexData();
				unsigned int* GetIndexData();

				void Draw();
				unsigned int vertexCountToRender();
				eae6320::cResult Release();

				void InitData(const std::vector<cGeometryVertex>& vertices, const std::vector<unsigned int> &triangleIndices);
				eae6320::cResult InitDevicePipeline();

#if defined( EAE6320_PLATFORM_D3D )
				cGeometryRenderTarget() { m_vertexBuffer = m_indexBuffer = nullptr; }
				eae6320::Graphics::cVertexFormat::Handle m_vertexFormat;
				ID3D11Buffer* m_vertexBuffer;
				ID3D11Buffer* m_indexBuffer;
#elif defined( EAE6320_PLATFORM_GL )
				cGeometryRenderTarget() { s_vertexBufferId = s_indexBufferId = s_vertexArrayId = 0; }
				GLuint s_vertexBufferId;
				GLuint s_indexBufferId;
				GLuint s_vertexArrayId;
#endif
				std::vector<cGeometryVertex> m_vertices;
				std::vector<unsigned int> m_indices;

			};
		}
	}
}


