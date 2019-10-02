#include "cGeometry.h"
#include "PlatformIncludes.h"
#include "GraphicsEnv.h"
#include "sContext.h"
#include <Engine/Asserts/Asserts.h>
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


			eae6320::cResult cGeometryRenderTarget::InitData(std::string i_path)
			{
				auto result = eae6320::Results::Success;
				lua_State* luaState = nullptr;
				eae6320::cScopeGuard scopeGuard_onExit([&luaState]
					{
						if (luaState)
						{
							// If I haven't made any mistakes
							// there shouldn't be anything on the stack
							// regardless of any errors
							EAE6320_ASSERT(lua_gettop(luaState) == 0);

							lua_close(luaState);
							luaState = nullptr;
						}
					});

				{
					luaState = luaL_newstate();
					if (!luaState)
					{
						result = eae6320::Results::OutOfMemory;
						EAE6320_ASSERTF(false, "Failed to create a new Lua state");
						return result;
					}
				}
				const auto stackTopBeforeLoad = lua_gettop(luaState);
				{
					const auto luaResult = luaL_loadfile(luaState, i_path.c_str());
					if (luaResult != LUA_OK)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, lua_tostring(luaState, -1));
						// Pop the error message
						lua_pop(luaState, 1);
						return result;
					}
				}

				{
					constexpr int argumentCount = 0;
					constexpr int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
					constexpr int noMessageHandler = 0;
					const auto luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
					if (luaResult == LUA_OK)
					{
						// A well-behaved asset file will only return a single value
						const auto returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
						if (returnedValueCount == 1)
						{
							// A correct asset file _must_ return a table
							if (!lua_istable(luaState, -1))
							{
								result = eae6320::Results::InvalidFile;
								//std::string errorMessage = "Asset files must return a table (instead of a " + luaL_typename(luaState, -1) + ")";
								EAE6320_ASSERTF(false, "Asset files must return a table");
								// Pop the returned non-table value
								lua_pop(luaState, 1);
								return result;
							}
						}
						else
						{
							result = eae6320::Results::InvalidFile;
							EAE6320_ASSERTF(false, "Asset files must return a single table");
							/*std::cerr << "Asset files must return a single table (instead of "
								<< returnedValueCount << " values)" << std::endl;*/
								// Pop every value that was returned
							lua_pop(luaState, returnedValueCount);
							return result;
						}
					}
					else
					{
						result = eae6320::Results::InvalidFile;
						EAE6320_ASSERTF(false, "Read file failed");
						//std::cerr << lua_tostring(luaState, -1);
						// Pop the error message
						lua_pop(luaState, 1);
						return result;
					}
				}

				// If this code is reached the asset file was loaded successfully,
				// and its table is now at index -1

				// A scope guard is used to pop the asset as soon as the scope (this function) is left
				eae6320::cScopeGuard scopeGuard_popAssetTable([luaState]
					{
						lua_pop(luaState, 1);
					});
				if (!(result = LoadIndicesFromLua(*luaState)))
				{
					return result;
				}
				if (!(result = LoadVerticesFromLua(*luaState)))
				{
					return result;
				}
				UpdateData();
				return result;
			}

			eae6320::cResult cGeometryRenderTarget::LoadIndicesFromLua(lua_State& io_luaState)
			{
				auto result = eae6320::Results::Success;

				constexpr auto* const keyIndices = "indices";
				lua_pushstring(&io_luaState, keyIndices);
				lua_gettable(&io_luaState, -2);
				eae6320::cScopeGuard scopeGuard_popIndicesTable([&io_luaState]
					{
						lua_pop(&io_luaState, 1);
					});

				if (lua_istable(&io_luaState, -1))
				{
					const auto indicesCount = luaL_len(&io_luaState, -1);
					std::vector<unsigned int> faceIndices = { 0,0,0 };
					for (int i = 1; i <= indicesCount; ++i)
					{
						lua_pushinteger(&io_luaState, i);
						lua_gettable(&io_luaState, -2);
						eae6320::cScopeGuard scopeGuard_popIndicesIndex([&io_luaState]
							{
								lua_pop(&io_luaState, 1);
							});
						faceIndices[(i - 1) % 3] = static_cast<unsigned int>(lua_tonumber(&io_luaState, -1));
						if (i % 3 == 0)
						{
							AddFace(cGeometryIndexFace(faceIndices[0], faceIndices[1], faceIndices[2]));
						}

					}
				}
				else {
					result = eae6320::Results::InvalidFile;
					EAE6320_ASSERTF(false, "No Indices Table");
					return result;
				}
				return result;
			}

			eae6320::cResult cGeometryRenderTarget::LoadVerticesFromLua(lua_State& io_luaState)
			{
				auto result = eae6320::Results::Success;

				constexpr auto* const keyVertices = "vertices";
				lua_pushstring(&io_luaState, keyVertices);
				lua_gettable(&io_luaState, -2);
				eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
					{
						lua_pop(&io_luaState, 1);
					});

				if (lua_istable(&io_luaState, -1))
				{
					const auto verticesCount = luaL_len(&io_luaState, -1);
					for (int i = 1; i <= verticesCount; ++i)
					{
						lua_pushinteger(&io_luaState, i);
						lua_gettable(&io_luaState, -2);
						eae6320::cScopeGuard scopeGuard_popVerticesIndex([&io_luaState]
							{
								lua_pop(&io_luaState, 1);
							});
						std::vector<float> vertices;
						for (int j = 1; j <= 3; j++)
						{
							lua_pushinteger(&io_luaState, j);
							lua_gettable(&io_luaState, -2);
							vertices.push_back(static_cast<float>(lua_tonumber(&io_luaState, -1)));
							eae6320::cScopeGuard scopeGuard_poVerticePositionIndex([&io_luaState]
								{
									lua_pop(&io_luaState, 1);
								});
						}
						m_vertices.push_back(cGeometryVertex(vertices[0], vertices[1], vertices[2]));
					}
				}
				else {
					result = eae6320::Results::InvalidFile;
					EAE6320_ASSERTF(false, "No vertices table");
					return result;
				}
				return result;
			}

			unsigned int cGeometryRenderTarget::GetIndexCount()
			{
				return (unsigned int)m_indices.size();
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


