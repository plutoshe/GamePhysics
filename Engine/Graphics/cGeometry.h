#pragma once

#include <vector>

#include "cVertexFormat.h"
#include "PlatformIncludes.h"
#include <Engine/Assets/ReferenceCountedAssets.h>
#include <External/Lua/Includes.h>

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
				
				EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cGeometryRenderTarget);
				EAE6320_ASSETS_DECLAREREFERENCECOUNT();
				EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();
				using Handle = Assets::cHandle<cGeometryRenderTarget>;
				static std::map<std::string, Handle> s_hanlderMap;
				static Assets::cManager<cGeometryRenderTarget> s_manager;
				cResult CleanUp() { Release(); }

				eae6320::cResult LoadIndicesFromLua(lua_State& io_luaState);
				eae6320::cResult LoadVerticesFromLua(lua_State& io_luaState);
				static eae6320::cResult Load(const std::string& i_path, cGeometryRenderTarget*& o_geometry);
				unsigned int VertexBufferSize();
				unsigned int GetIndexCount();
				unsigned int IndexBufferSize();
				void AddFace(const cGeometryIndexFace &face);
				void AddIndices(int FaceNum, const std::vector<unsigned int> &triangleIndices);
				void AddVetices(int vertexNum, const std::vector<cGeometryVertex>& vertices);
				void SetIndicesByFaces(std::vector<cGeometryIndexFace>& i_faces);
				void SetIndices(std::vector<unsigned int>& i_indices);
				void SetVertices(std::vector<cGeometryVertex>& i_vertices);
				void UpdateData();
				cGeometryVertex* GetVertexData();
				unsigned int* GetIndexData();
				unsigned int vertexCountToRender();
				void InitData(const std::vector<cGeometryVertex>& vertices, const std::vector<unsigned int> &triangleIndices);
				eae6320::cResult InitData(std::string i_path);
#if defined( EAE6320_PLATFORM_D3D )
				cGeometryRenderTarget() { m_isInitialized = false; m_isUpdateData = false; m_vertexBuffer = m_indexBuffer = nullptr; }
				ID3D11Buffer* m_vertexBuffer = nullptr;
				ID3D11Buffer* m_indexBuffer = nullptr;
#elif defined( EAE6320_PLATFORM_GL )
				cGeometryRenderTarget() { m_isInitialized = false; m_isUpdateData = false; m_vertexBufferId = m_indexBufferId = m_vertexArrayId = 0; }
				GLuint m_vertexBufferId = 0;
				GLuint m_indexBufferId = 0;
				GLuint m_vertexArrayId = 0;
#endif

				bool m_isInitialized;
				bool m_isUpdateData;
				std::vector<cGeometryVertex> m_vertices;
				std::vector<unsigned int> m_indices;

				eae6320::cResult Release();
				~cGeometryRenderTarget() { Release(); }
			};

			class cGeometry {
			public:
				std::string m_path;
				cGeometryRenderTarget::Handle m_handler;
				cGeometry(const cGeometry& i_geometry) { m_path = i_geometry.m_path; m_handler = i_geometry.m_handler; }
				cGeometry() { m_path = ""; }
				~cGeometry() { }
				cGeometry(std::string i_path) { m_path = i_path; }
				eae6320::cResult InitDevicePipeline();
				eae6320::cResult Load();
				void Draw();
			};
		}
	}
}


