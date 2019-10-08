// Includes
//=========

#include "cMyGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/Graphics/cRenderObject.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cGeometry.h>
#include <vector>
#define M_PI 3.1415926f
#define Deg2Rad(x) (x * M_PI / 180.f)
#define Rad2Deg(x) (x * 180.f / M_PI)
// Inherited Implementation
//=========================

// Run
//----

void eae6320::cMyGame::UpdateSimulationBasedOnInput()
{
	// Object movement
	Math::sVector objectVelocity(0, 0, 0);
	float objectSpeed = 2.f;
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Up))
	{
		objectVelocity.y += objectSpeed;
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Down))
	{
		objectVelocity.y -= objectSpeed;
	}
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Left))
	{
		objectVelocity.x -= objectSpeed;
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Right))
	{
		objectVelocity.x += objectSpeed;
	}
	if (m_gameObjects.size() > 0)
	{
		m_gameObjects[0].m_rigidBodyStatue.velocity = objectVelocity;
	}

	// camera Movement;
	Math::sVector cameraVelocity(0, 0, 0), cameraAngularVelocity(0, 0, 0);
	float cameraSpeed = 5.f;
	float cameraAngularSpeed = 1.f;
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::W))
	{
		cameraVelocity.z += cameraSpeed;
	}
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::S))
	{
		cameraVelocity.z -= cameraSpeed;
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::A))
	{
		cameraVelocity.x -= cameraSpeed;
	}
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::D))
	{
		cameraVelocity.x += cameraSpeed;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Q))
	{
		cameraAngularVelocity.y += cameraAngularSpeed;
		
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::E))
	{
		cameraAngularVelocity.y -= cameraAngularSpeed;
	}
	m_camera.SetVelocityInCameraAxis(cameraVelocity);
	m_camera.SetAngularVelocity(cameraAngularVelocity);
}

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
		SetBackgroundColor(0.5f, 0.0f, 0.5f, 1.f);
	}
	else
	{
		SetBackgroundColor(0.7f, 0.7f, 0.7f, 1.f);
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::J))
	{
		if (!m_isJPressed)
		{
			DeleteGameObjectById((int)m_gameObjects.size() - 1);
			m_isJPressed = true;
		}
	}
	else if (m_isJPressed)
	{
		m_isJPressed = false;
		eae6320::Graphics::Geometry::cGeometry geometryB;
		geometryB.m_path = "data/geometries/object2.bin";
		geometryB.Load();

		eae6320::Graphics::Effect* effectB;
		eae6320::Graphics::Effect::Factory(effectB);
		effectB->SetVertexShaderPath("data/shaders/vertex/standard.shader");
		effectB->SetFragmentShaderPath("data/shaders/fragment/vertexColor.shader");
		
		AddGameObject(Application::GameObject(eae6320::Graphics::RenderObject(geometryB, effectB)));
		effectB->DecrementReferenceCount();
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::L))
	{
		if (!m_isLPressed)
		{
			//m_gameObjects[0].m_renderObject.m_geometry->SetIndices(std::vector<unsigned int>{ 0, 1, 2 });
			m_isLPressed = true;
		}
	}
	else if (m_isLPressed)
	{
		m_isLPressed = false;
		//m_gameObjects[0].m_renderObject.m_geometry->SetIndices(std::vector<unsigned int>{ 0, 1, 2, 1, 3, 2 });
	}
	/*if (m_gameObjects.size() > 0) 
	{
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::K))
		{
			m_effectChangeB->SetToPointer(m_gameObjects[0].m_renderObject.m_effect);
		}
		else
		{
			m_effectChangeA->SetToPointer(m_gameObjects[0].m_renderObject.m_effect);
		}
	}*/
}

// Initialization / Clean Up
//--------------------------


eae6320::cResult eae6320::cMyGame::Initialize()
{
	m_camera.m_AspectRatio = 1;
	m_camera.m_rigidBodyStatue.position = Math::sVector(0, 0, 10);


	m_camera.m_FOV = Deg2Rad(45);
	m_camera.m_ZFarPlane = 100.f;
	m_camera.m_ZNearPlane = 0.01f;

	

	eae6320::Graphics::Geometry::cGeometry geometryA("data/geometries/object2.bin");
	eae6320::Graphics::Geometry::cGeometry geometryB("data/geometries/object3.bin");
	eae6320::Graphics::Geometry::cGeometry geometryC("data/geometries/object1.bin");
	auto resultGeometryA = geometryA.Load();
	auto resultGeometryB = geometryB.Load();
	auto resultGeometryC = geometryC.Load();
	
	eae6320::Graphics::Effect* effectA, * effectB;
	eae6320::Graphics::Effect::Factory(effectA);
	eae6320::Graphics::Effect::Factory(effectB);
	eae6320::Graphics::Effect::Factory(m_effectChangeA);
	eae6320::Graphics::Effect::Factory(m_effectChangeB);


	effectA->SetVertexShaderPath("data/shaders/vertex/standard.shader");
	effectA->SetFragmentShaderPath("data/shaders/fragment/vertexColor.shader");
	effectB->SetVertexShaderPath("data/shaders/vertex/standard.shader");
	effectB->SetFragmentShaderPath("data/shaders/fragment/vertexColor.shader");
	m_effectChangeA->SetVertexShaderPath("data/shaders/vertex/standard.shader");
	m_effectChangeA->SetFragmentShaderPath("data/shaders/fragment/blue.shader");
	m_effectChangeB->SetVertexShaderPath("data/shaders/vertex/standard.shader");
	m_effectChangeB->SetFragmentShaderPath("data/shaders/fragment/change_color.shader");
	std::vector<Application::GameObject> objs;
	if (eae6320::Results::Success)
	{
		int a = 0;
	}
	if (resultGeometryA)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryA, effectB)));
	}
	if (resultGeometryC)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryC, effectB)));
	}
	if (resultGeometryB)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryB, effectB)));
	}

	SetGameObjects(objs);

	effectA->DecrementReferenceCount();
	effectB->DecrementReferenceCount();
	return Results::Success;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{
	
	Application::cbApplication::CleanUp();
	if (m_effectChangeA)
	{
		m_effectChangeA->DecrementReferenceCount();
	}
	if (m_effectChangeB)
	{
		m_effectChangeB->DecrementReferenceCount();
	}
	m_effectChangeA = nullptr; m_effectChangeB = nullptr;
	
	return Results::Success;
}
