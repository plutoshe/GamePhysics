#include "cEffectBuilder.h"

#include <Engine/Graphics/cEffect.h>
#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <External/Lua/Includes.h>
#include <fstream>
// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cEffectBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = eae6320::Results::Success;
	std::string errorMessage;
	eae6320::Graphics::EffectAsset loadedEffect;
	eae6320::Assets::cEffectBuilder::InitData(m_path_source, loadedEffect);
	std::ofstream outfile(m_path_target, std::ofstream::binary);
	if (!outfile.is_open())
	{
		OutputErrorMessageWithFileInfo(m_path_source, "no invalid geometry output path!");

		result = eae6320::Results::Failure;
		return result;
	}

	loadedEffect.m_vertexShaderPath = "data/" + loadedEffect.m_vertexShaderPath + '\0';
	loadedEffect.m_fragmentShaderPath = "data/" + loadedEffect.m_fragmentShaderPath + '\0';
	outfile.write(loadedEffect.m_vertexShaderPath.c_str(), loadedEffect.m_vertexShaderPath.size());
	outfile.write(loadedEffect.m_fragmentShaderPath.c_str(), loadedEffect.m_fragmentShaderPath.size());
	
	outfile.close();
	return result;
}


eae6320::cResult eae6320::Assets::cEffectBuilder::InitData(std::string i_path, eae6320::Graphics::EffectAsset& o_effect)
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
	{
		constexpr auto* const keyVertexShader = "vertex";
		lua_pushstring(luaState, keyVertexShader);
		lua_gettable(luaState, -2);
		o_effect.m_vertexShaderPath = lua_tostring(luaState, -1);
		eae6320::cScopeGuard scopeGuard_popIndicesTable([luaState]
		{
			lua_pop(luaState, 1);
		});
	}
	{
		constexpr auto* const keyFragmentShader = "fragment";
		lua_pushstring(luaState, keyFragmentShader);
		lua_gettable(luaState, -2);
		o_effect.m_fragmentShaderPath = lua_tostring(luaState, -1);
		eae6320::cScopeGuard scopeGuard_popFragment([luaState]
		{
			lua_pop(luaState, 1);
		});
	}

	return result;
}