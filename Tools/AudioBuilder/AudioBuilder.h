#pragma once

// Includes
//=========

#include <Tools/AssetBuildLibrary/cbBuilder.h>
//#include <Engine/Graphics/Configuration.h>

// Class Declaration
//==================

namespace eae6320
{
	namespace Assets
	{
		class AudioBuilder : public cbBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			virtual cResult Build(const std::vector<std::string>& i_arguments) override;

			// Implementation
			//===============

		private:

			// Build
			//------

			// I don't think we'll need this.
			//cResult Build();
		};
	}
}