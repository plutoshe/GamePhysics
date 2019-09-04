#pragma once

#include <vector>

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

			class cGeometryFace
			{
			public:
				// default store as right-handed
				cGeometryVertex m_v0, m_v1, m_v2;
				cGeometryFace() {}
				cGeometryFace(cGeometryVertex v0, cGeometryVertex v1, cGeometryVertex v2) 
				{
					m_v0 = v0; m_v1 = v1; m_v2 = v2;
				}
				cGeometryVertex* GetGeometryVertices();
			};

			class cGeometryRenderTarget
			{
			public:
				cGeometryRenderTarget() {}
				unsigned int BufferSize();
				void AddFace(cGeometryFace face);
				cGeometryVertex* GetVertexData();
				std::vector<cGeometryVertex> vertices;
			};
		}
	}
}


