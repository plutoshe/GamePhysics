#include "cGeometryBuilder.h"

#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <External/Lua/Includes.h>
#include <fstream>
// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cGeometryBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = eae6320::Results::Success;
	std::string errorMessage;
	eae6320::Graphics::Geometry::cGeometryRenderTarget loadedGeometry;
	eae6320::Assets::cGeometryBuilder::InitData(m_path_source, loadedGeometry);
	std::ofstream outfile(m_path_target, std::ofstream::binary | std::ios::out);
	if (!outfile.is_open())
	{
		OutputErrorMessageWithFileInfo(m_path_source, "no invalid geometry output path!");
		
		result = eae6320::Results::Failure;
		return result;
	}
	size_t vertexNum = loadedGeometry.m_vertices.size();
	size_t indexNum = loadedGeometry.m_indices.size();
	outfile.write(reinterpret_cast<const char*>(&vertexNum), sizeof(vertexNum));
	outfile.write(reinterpret_cast<const char*>(&loadedGeometry.m_vertices[0]), loadedGeometry.m_vertices.size() * sizeof(eae6320::Graphics::Geometry::cGeometryVertex));
	outfile.write(reinterpret_cast<const char*>(&indexNum), sizeof(size_t));
	outfile.write(reinterpret_cast<const char*>(&loadedGeometry.m_indices[0]), loadedGeometry.m_indices.size() * sizeof(unsigned int));
	outfile.close();
	return result;
}
eae6320::cResult eae6320::Assets::cGeometryBuilder::LoadIndicesFromLua(lua_State& io_luaState, eae6320::Graphics::Geometry::cGeometryRenderTarget& o_geometry)
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
				o_geometry.AddFace(eae6320::Graphics::Geometry::cGeometryIndexFace(faceIndices[0], faceIndices[1], faceIndices[2]));
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

eae6320::cResult eae6320::Assets::cGeometryBuilder::LoadVerticesFromLua(lua_State& io_luaState, eae6320::Graphics::Geometry::cGeometryRenderTarget& o_geometry)
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
			eae6320::cScopeGuard scopeGuard_popVerticesTable([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});
			eae6320::Graphics::Geometry::cGeometryVertex currentVertex;
			if (lua_istable(&io_luaState, -1))
			{
				{
					lua_pushstring(&io_luaState, "position");
					lua_gettable(&io_luaState, -2);
					eae6320::cScopeGuard scopeGuard_popVerticesPosition([&io_luaState]
					{
						lua_pop(&io_luaState, 1);
					});
					if (lua_istable(&io_luaState, -1))
					{
						for (int j = 1; j <= 3; j++)
						{
							lua_pushinteger(&io_luaState, j);
							auto result = lua_gettable(&io_luaState, -2);
							auto currentValue = static_cast<float>(lua_tonumber(&io_luaState, -1));
							eae6320::cScopeGuard scopeGuard_popVertexPos([&io_luaState]
							{
								lua_pop(&io_luaState, 1);
							});
							if (result > 0)
							{
								switch (j)
								{
								case 1: currentVertex.m_x = currentValue; break;
								case 2: currentVertex.m_y = currentValue; break;
								case 3: currentVertex.m_z = currentValue; break;
								}
							}
						}
					}
				}
				{
					lua_pushstring(&io_luaState, "color");
					lua_gettable(&io_luaState, -2);
					eae6320::cScopeGuard scopeGuard_popVerticesColor([&io_luaState]
					{
						lua_pop(&io_luaState, 1);
					});
					if (lua_istable(&io_luaState, -1))
					{
						for (int j = 1; j <= 4; j++)
						{
							lua_pushinteger(&io_luaState, j);
							auto result = lua_gettable(&io_luaState, -2);
							auto currentValue = static_cast<uint8_t>(lua_tonumber(&io_luaState, -1));
							eae6320::cScopeGuard scopeGuard_popVertexColor([&io_luaState]
							{
								lua_pop(&io_luaState, 1);
							});
							if (result > 0)
							{
								switch (j)
								{
								case 1: currentVertex.m_r = currentValue; break;
								case 2: currentVertex.m_g = currentValue; break;
								case 3: currentVertex.m_b = currentValue; break;
								case 4: currentVertex.m_a = currentValue; break;
								}
							}
						}

					}
				}
				o_geometry.m_vertices.push_back(currentVertex);
			}

		}
	}
	else {
		result = eae6320::Results::InvalidFile;
		EAE6320_ASSERTF(false, "No vertices table");
		return result;
	}
	return result;
}


eae6320::cResult eae6320::Assets::cGeometryBuilder::InitData(std::string i_path, eae6320::Graphics::Geometry::cGeometryRenderTarget& o_geometry)
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
			auto errorMessage = lua_tostring(luaState, -1);
			EAE6320_ASSERTF(false, "cannot load file");
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
	if (!(result = LoadIndicesFromLua(*luaState, o_geometry)))
	{
		return result;
	}
	if (!(result = LoadVerticesFromLua(*luaState, o_geometry)))
	{
		return result;
	}
	return result;
}