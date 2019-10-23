#pragma once

#include <Tools/AssetBuildLibrary/cbBuilder.h>
#include <Engine/Graphics/Configuration.h>
#include <Engine/Graphics/cGeometry.h>
namespace eae6320
{
	namespace Assets
	{
		class cGeometryBuilder : public cbBuilder
		{
		private:
			virtual cResult Build(const std::vector<std::string>& i_arguments) override;
			virtual eae6320::cResult LoadIndicesFromLua(lua_State& io_luaState, eae6320::Graphics::Geometry::cGeometryRenderTarget &o_geometry);
			virtual eae6320::cResult LoadVerticesFromLua(lua_State& io_luaState, eae6320::Graphics::Geometry::cGeometryRenderTarget& o_geometry);
			virtual eae6320::cResult InitData(std::string i_path, eae6320::Graphics::Geometry::cGeometryRenderTarget& o_geometry);
		};
	}
}

