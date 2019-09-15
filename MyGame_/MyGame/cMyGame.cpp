// Includes
//=========

#include "cMyGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/Graphics/cRenderObject.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cGeometry.h>
#include <vector>

// Inherited Implementation
//=========================

// Run
//----

void eae6320::cMyGame::UpdateBasedOnInput()
{
	// Is the user pressing the ESC key?
	if ( UserInput::IsKeyPressed( UserInput::KeyCodes::Escape ) )
	{
		// Exit the application
		const auto result = Exit( EXIT_SUCCESS );
		EAE6320_ASSERT( result );
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::I))
	{
		SetBackgroundColor(std::vector<float>{1.0f, 0.f, 0.f, 1.f});
	}
	else
	{
		SetBackgroundColor(std::vector<float>{1.0f, 1.f, 0.f, 1.f});
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::J))
	{
		SetBackgroundColor(std::vector<float>{1.0f, 0.f, 0.f, 1.f});
	}
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cMyGame::Initialize()
{
	std::vector<eae6320::Graphics::Geometry::cGeometryVertex> verticesA{
				eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 0.0f, 0.0f),
				eae6320::Graphics::Geometry::cGeometryVertex(1.0f, 0.0f, 0.0f),
				eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 1.0f, 0.0f),
				eae6320::Graphics::Geometry::cGeometryVertex(1.0f, 1.0f, 0.0f),
	};
	std::vector<unsigned int> indicesA{ 0, 1, 2, 1, 3, 2 };

	std::vector<eae6320::Graphics::Geometry::cGeometryVertex> verticesB{
		eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, -1.0f, 0.0f),
		eae6320::Graphics::Geometry::cGeometryVertex(0.0f, -1.0f, 0.0f),
		eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, 0.0f, 0.0f),
		eae6320::Graphics::Geometry::cGeometryVertex(-0.3f, -0.3f, 0.0f),
	};

	eae6320::Graphics::Geometry::cGeometryRenderTarget geometryA, geometryB;
	geometryA.InitData(verticesA, indicesA);
	geometryB.InitData(verticesB, indicesA);


	eae6320::Graphics::Effect effectA, effectB;
	effectA.SetVertexShaderPath("data/shaders/vertex/standard.shader");
	effectA.SetFragmentShaderPath("data/shaders/fragment/change_color.shader");
	effectB.SetVertexShaderPath("data/shaders/vertex/standard.shader");
	effectB.SetFragmentShaderPath("data/shaders/fragment/standard.shader");

	SetRenderObjects(std::vector<Graphics::RenderObject>{Graphics::RenderObject(geometryA, effectA), Graphics::RenderObject(geometryB, effectB)});
	return Results::Success;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{
	return Results::Success;
}
