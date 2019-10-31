#pragma once

#include <Tools/AssetBuildLibrary/cbBuilder.h>
#include <Engine/Graphics/Configuration.h>
#include <Engine/Graphics/cEffect.h>
namespace eae6320
{
	namespace Assets
	{
		class cEffectBuilder : public cbBuilder
		{
		private:
			virtual cResult Build(const std::vector<std::string>& i_arguments) override;
			virtual eae6320::cResult InitData(std::string i_path, eae6320::Graphics::EffectAsset& o_effect);
		};
	}
}

