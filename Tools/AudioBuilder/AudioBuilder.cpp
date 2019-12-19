#include "AudioBuilder.h"
#include <Engine/Platform/Platform.h>
#include <Tools/AssetBuildLibrary/Functions.h>

#include<Engine/Asserts/Asserts.h>
#include <Engine/3DAudio/AudioSource.h>
#include <External/Lua/Includes.h>
#include <iostream>
#include <Engine/Math/sVector.h>
#include <Engine/Logging/Logging.h>
//#include <sstream>
#include <fstream>

#include <Engine/ScopeGuard/cScopeGuard.h>

// Lua Parsing Declarations
namespace
{
	eae6320::cResult LoadAsset(const char* const i_path, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage);
	eae6320::cResult LoadTableValues(lua_State& io_luaState, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage);

	eae6320::cResult LoadTableValues_filepath(lua_State& io_luaState, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage);
	eae6320::cResult LoadTableValues_position(lua_State& io_luaState, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage);
	eae6320::cResult LoadTableValues_position_values(lua_State& io_luaState, float* values, std::string* errMessage);
}


eae6320::cResult eae6320::Assets::AudioBuilder::Build(const std::vector<std::string>& i_arguments)
{
	std::string errorMsg;
	auto result = Results::Success;

	//Audio3D::AudioInitParams initParams;

	////OutputErrorMessage("Starting Build.");
	//// Load the human-readable data
	//{
	//	std::string errorMessage;
	//	if (!(result = LoadAsset(m_path_source, initParams, &errorMessage)))
	//	{
	//		std::cout << errorMessage.c_str();
	//		//	EAE6320_ASSERTF(false, errorMessage.c_str());
	//		OutputErrorMessageWithFileInfo(m_path_source, errorMessage.c_str());
	//		//OutputErrorMessage("load asset failed");

	//		return result;
	//	}
	//	//OutputErrorMessage("Got vertex count as %d", initParams.m_nVertices);
	//}
	//
	//// write to the binary file
	//{
	//	//OutputErrorMessage("Writing to binary file.");
	//	std::ofstream outfile(m_path_target, std::ofstream::binary);

	//	if (!outfile) {
	//		OutputErrorMessage("Cannot open binary file for write.");
	//		return Results::Failure;
	//	}

	//	// this is just making things cleaner and easier to read
	//	eae6320::Math::sVector position = initParams.position;

	//	char* path = initParams.filename;
	//	
	//	if (!outfile.good()) {
	//		OutputErrorMessage("Error occurred at writing time!");
	//		return Results::Failure;
	//	}
	//	//outfile.write(reinterpret_cast<char*> (&position.x), sizeof(float));
	//	//outfile.write(reinterpret_cast<char*> (&position.y), sizeof(float));
	//	//outfile.write(reinterpret_cast<char*> (&position.z), sizeof(float));
	//	outfile.write(reinterpret_cast<char*> (path), strlen(path)+1);

	//	outfile.close();
	//	if (!outfile.good()) {
	//		OutputErrorMessage("Error occurred at writing time!");
	//		return Results::Failure;
	//	}

	//	//free(path);
	//	initParams.filename = nullptr;
	//}

	
	result = eae6320::Platform::CopyFile(m_path_source, m_path_target, false, true, &errorMsg);
	if(!result)
		OutputErrorMessageWithFileInfo(m_path_source, errorMsg.c_str());
	

	return result;
}

// Lua Parsing File Definitions
// ============================ here it comes

namespace
{
	eae6320::cResult LoadAsset(const char* const i_path, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage)
	{
		auto result = eae6320::Results::Success;

		// Create a new Lua state
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
				std::cerr << "Failed to create a new Lua state" << std::endl;
				errMessage->append("Failed to create a new Lua state");
				return result;
			}
		}

		// Load the asset file as a "chunk",
		// meaning there will be a callable function at the top of the stack
		const auto stackTopBeforeLoad = lua_gettop(luaState);
		{
			const auto luaResult = luaL_loadfile(luaState, i_path);
			if (luaResult != LUA_OK)
			{
				result = eae6320::Results::Failure;
				std::cerr << lua_tostring(luaState, -1) << std::endl;
				errMessage->append(lua_tostring(luaState, -1));
				// Pop the error message
				lua_pop(luaState, 1);
				return result;
			}
		}
		// Execute the "chunk", which should load the asset
		// into a table at the top of the stack
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
						std::cerr << "Asset files must return a table (instead of a " <<
							luaL_typename(luaState, -1) << ")" << std::endl;
						errMessage->append("Asset files must return a table");
						// Pop the returned non-table value
						lua_pop(luaState, 1);
						return result;
					}
				}
				else
				{
					result = eae6320::Results::InvalidFile;
					std::cerr << "Asset files must return a single table (instead of " <<
						returnedValueCount << " values)" << std::endl;
					errMessage->append("Asset files must return a single table");
					// Pop every value that was returned
					lua_pop(luaState, returnedValueCount);
					return result;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				std::cerr << lua_tostring(luaState, -1) << std::endl;
				// Pop the error message
				lua_pop(luaState, 1);
				return result;
			}
		}

		// If this code is reached the asset file was loaded successfully,
		// and its table is now at index -1
		eae6320::cScopeGuard scopeGuard_popAssetTable([luaState]
		{
			lua_pop(luaState, 1);
		});
		result = LoadTableValues(*luaState, o_initParams, errMessage);
		eae6320::Logging::OutputMessage("Filepath is %s", o_initParams.filename);
		eae6320::Logging::OutputMessage("Position count is %f %f %f", o_initParams.position.x, o_initParams.position.y, o_initParams.position.z);

		return result;
	}

	eae6320::cResult LoadTableValues(lua_State& io_luaState, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage)
	{
		auto result = eae6320::Results::Success;

		if (!(result = LoadTableValues_filepath(io_luaState, o_initParams, errMessage)))
		{
			return result;
		}
		if (!(result = LoadTableValues_position(io_luaState, o_initParams, errMessage)))
		{
			return result;
		}

		return result;
	}

	eae6320::cResult LoadTableValues_filepath(lua_State& io_luaState, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage)
	{
		auto result = eae6320::Results::Success;

		// Get the value of "file"
		{

			constexpr auto* const key = "file";
			lua_pushstring(&io_luaState, key);

			{
				constexpr int currentIndexOfTable = -2;
				lua_gettable(&io_luaState, currentIndexOfTable);
			}

			eae6320::cScopeGuard scopeGuard_popName([&io_luaState]
			{
				// The value from "name" should currently be at -1 and the asset table at -2;
				// In order to restore the stack to the way it was when we entered this scope
				// we need to pop the value from "name" off the stack:
				lua_pop(&io_luaState, 1);
			});


			if (lua_isnil(&io_luaState, -1))
			{
				result = eae6320::Results::InvalidFile;
				std::cerr << "No value for \"" << key << "\" was found in the asset table" << std::endl;
				// The scope guard will automatically pop the stack
				// and the calling function can then deal with the error
				// (and it knows that the stack will be in the same state regardless of success or failure of the function)
				return result;
			}

			if (lua_type(&io_luaState, -1) != LUA_TSTRING)
			{
				result = eae6320::Results::InvalidFile;
				std::cerr << "The value for \"" << key << "\" must be a string "
					"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
				return result;
			}

			// Now we know that the value stored in the table at the key is valid:
			{
				const auto* const value = lua_tostring(&io_luaState, -1);
				// You can now do whatever you want with the value.
				// NOTE! The string is a pointer to a char array,
				// which means that Lua owns the memory.
				// You should do whatever you need with the value
				// _before_ popping the value from the stack.
				// If you can't use the value right away,
				// then either copy it to your own char array or store it in a std::string.
				// (THIS IS ONE OF THE MOST COMMON MISTAKES DEALING WITH LUA IN THIS CLASS!)
				const std::string myCopyOfTheValue = value;
				std::cout << "The value for \"" << key << "\" is \"" << value << "\"" << std::endl;
				size_t size = strlen(value);
				o_initParams.filename = (char*)malloc(sizeof(char) * size);
				strcpy(o_initParams.filename, value);
			}
		}

		return result;
	}
	

	eae6320::cResult LoadTableValues_position(lua_State& io_luaState, eae6320::Audio3D::AudioInitParams& o_initParams, std::string* errMessage)
	{
		auto result = eae6320::Results::Success;

		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1:
		constexpr auto* const key = "position";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);
		// It can be hard to remember where the stack is at
		// and how many values to pop.
		// There are two ways that I suggest making it easier to keep track of this:
		//	1) Use scope guards to pop things automatically
		//	2) Call a different function when you are at a new level
		// Right now we know that we have an original table at -2,
		// and a new one at -1,
		// and so we _know_ that we always have to pop at least _one_
		// value before leaving this function
		// (to make the original table be back to index -1).
		// We can create a scope guard immediately as soon as the new table has been pushed
		// to guarantee that it will be popped when we are done with it:
		eae6320::cScopeGuard scopeGuard_popTextures([&io_luaState]
		{
			lua_pop(&io_luaState, 1);
		});
		// Additionally, I try not to do any further stack manipulation in this function
		// and call other functions that assume the "textures" table is at -1
		// but don't know or care about the rest of the stack
		if (lua_istable(&io_luaState, -1))
		{
			float values[3];
			if (!(result = LoadTableValues_position_values(io_luaState, values, errMessage)))
			{
				return result;
			}
			o_initParams.position.x = values[0];
			o_initParams.position.y = values[1];
			o_initParams.position.z = values[2];
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			//std::cerr << "The value at \"" << key << "\" must be a table "
			//	"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
			errMessage->append("The value at vertices wasn't the right type.");
			std::cout << "whhoops";
			return result;
		}

		return result;
	}

	eae6320::cResult LoadTableValues_position_values(lua_State& io_luaState, float* values, std::string* errMessage)
	{
		auto result = eae6320::Results::Success;

		//const auto textureCount = 3;//luaL_len(&io_luaState, -1);
		//for (int i = 1; i <= textureCount; ++i)
		//{
		//	lua_pushinteger(&io_luaState, i);
		//	lua_gettable(&io_luaState, -2);
		//	eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
		//	{
		//		lua_pop(&io_luaState, 1);
		//	});
		//	// so at this point, the vertex table (the table for each vertex)
		//	// is at -1
		//	//std::cout << "\t" << lua_tointeger(&io_luaState, -1) << std::endl;
		//	eae6320::Logging::OutputMessage(lua_tostring(&io_luaState, -1));
		//	// NOW STORE THESE SOMEWHERE. BUT IT WORKS!!!!
		//}

		// for x
		{
			lua_pushinteger(&io_luaState, 1);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});
			//eae6320::Logging::OutputMessage("%f", (float)lua_tonumber(&io_luaState, -1) );
			*values = (uint16_t)lua_tonumber(&io_luaState, -1);
			std::cout << "x=" << *values;
		}
		// for y
		{
			lua_pushinteger(&io_luaState, 2);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});

			*(values + 1) = (uint16_t)lua_tonumber(&io_luaState, -1);
		}
		// for z
		{
			lua_pushinteger(&io_luaState, 3);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});

			*(values + 2) = (uint16_t)lua_tonumber(&io_luaState, -1);
		}

		return result;
	}
}