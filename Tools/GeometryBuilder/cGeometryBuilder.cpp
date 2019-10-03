#include "cGeometryBuilder.h"

#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Platform/Platform.h>

// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cGeometryBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = eae6320::Results::Success;
	std::string errorMessage;
	if (!(result = eae6320::Platform::CopyFile(m_path_source, m_path_target, false, true, &errorMessage))) {
		OutputErrorMessageWithFileInfo(m_path_source, errorMessage.c_str());
		return Results::Failure;
	}
	return result;
}
