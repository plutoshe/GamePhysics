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
		if (!m_isJPressed)
		{
			DeleteRenderObjectById((int)m_renderObjects.size() - 1);
			m_isJPressed = true;
		}
	}
	else if (m_isJPressed)
	{
		m_isJPressed = false;
		std::vector<eae6320::Graphics::Geometry::cGeometryVertex> verticesB{
			eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, -1.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(-0.4f, -1.0f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, -0.4f, 0.0f),
			eae6320::Graphics::Geometry::cGeometryVertex(-0.3f, -0.3f, 0.0f),
		};
		std::vector<unsigned int> indicesB{ 0, 1, 2, 1, 3, 2 };
		eae6320::Graphics::Geometry::cGeometryRenderTarget* geometryB;
		eae6320::Graphics::Geometry::cGeometryRenderTarget::Factory(geometryB);
		geometryB->InitData(verticesB, indicesB);
		eae6320::Graphics::Effect* effectB;
		eae6320::Graphics::Effect::Factory(effectB);
		effectB->SetVertexShaderPath("data/shaders/vertex/standard.shader");
		effectB->SetFragmentShaderPath("data/shaders/fragment/standard.shader");
		
		AddRenderObject(Graphics::RenderObject(geometryB, effectB));
		geometryB->DecrementReferenceCount();
		effectB->DecrementReferenceCount();
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::K))
	{
		m_effectChangeA->SetToPointer(m_renderObjects[0].m_effect);
	}
	else
	{
		m_effectChangeB->SetToPointer(m_renderObjects[0].m_effect);
	}
}

// Initialization / Clean Up
//--------------------------


eae6320::cResult eae6320::cMyGame::Initialize()
{
	std::vector<eae6320::Graphics::Geometry::cGeometryVertex> verticesA{
				eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 0.0f, 0.0f),
				eae6320::Graphics::Geometry::cGeometryVertex(0.5f, 0.0f, 0.0f),
				eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 0.5f, 0.0f),
				eae6320::Graphics::Geometry::cGeometryVertex(0.5f, 0.5f, 0.0f),
	};
	std::vector<unsigned int> indicesA{ 0, 1, 2, 1, 3, 2 };

	std::vector<eae6320::Graphics::Geometry::cGeometryVertex> verticesB{
		eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, -1.0f, 0.0f),
		eae6320::Graphics::Geometry::cGeometryVertex(-0.4f, -1.0f, 0.0f),
		eae6320::Graphics::Geometry::cGeometryVertex(-1.0f, -0.4f, 0.0f),
		eae6320::Graphics::Geometry::cGeometryVertex(-0.3f, -0.3f, 0.0f),
	};

	eae6320::Graphics::Geometry::cGeometryRenderTarget* geometryA;
	eae6320::Graphics::Geometry::cGeometryRenderTarget* geometryB;
	eae6320::Graphics::Geometry::cGeometryRenderTarget::Factory(geometryA);
	eae6320::Graphics::Geometry::cGeometryRenderTarget::Factory(geometryB);
	geometryA->InitData(verticesA, indicesA);
	geometryB->InitData(verticesB, indicesA);
	geometryA->InitDevicePipeline();
	geometryB->InitDevicePipeline();
	eae6320::Graphics::Effect* effectA, * effectB;
	eae6320::Graphics::Effect::Factory(effectA);
	eae6320::Graphics::Effect::Factory(effectB);
	eae6320::Graphics::Effect::Factory(m_effectChangeA);
	eae6320::Graphics::Effect::Factory(m_effectChangeB);


	effectA->SetVertexShaderPath("data/shaders/vertex/standard.shader");
	effectA->SetFragmentShaderPath("data/shaders/fragment/change_color.shader");
	effectB->SetVertexShaderPath("data/shaders/vertex/standard.shader");
	effectB->SetFragmentShaderPath("data/shaders/fragment/standard.shader");
	m_effectChangeA->SetVertexShaderPath("data/shaders/vertex/standard.shader");
	m_effectChangeA->SetFragmentShaderPath("data/shaders/fragment/blue.shader");
	m_effectChangeB->SetVertexShaderPath("data/shaders/vertex/expand2times.shader");
	m_effectChangeB->SetFragmentShaderPath("data/shaders/fragment/change_color.shader");
	SetRenderObjects(std::vector<Graphics::RenderObject>{Graphics::RenderObject(geometryA, effectA), Graphics::RenderObject(geometryB, effectB)});
	geometryA->DecrementReferenceCount();
	geometryB->DecrementReferenceCount();

	effectA->DecrementReferenceCount();
	effectB->DecrementReferenceCount();
	//SetRenderObjects(std::vector<Graphics::RenderObject>{Graphics::RenderObject(geometryA, effectA), Graphics::RenderObject(geometryB, effectB)});
	return Results::Success;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{
	
	Application::cbApplication::CleanUp();
	m_effectChangeA->DecrementReferenceCount(); m_effectChangeB->DecrementReferenceCount();
	m_effectChangeA = nullptr; m_effectChangeB = nullptr;
	
	return Results::Success;
}
