// Includes
//=========

#include "cMyGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/Graphics/cRenderObject.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cGeometry.h>
#include <vector>
#include <Engine/PhysicsSystem/PhysicsSystem.h>
#define M_PI 3.1415926f
#define Deg2Rad(x) (x * M_PI / 180.f)
#define Rad2Deg(x) (x * 180.f / M_PI)
// Inherited Implementation
//=========================

// Run
//----


eae6320::cResult eae6320::cMyGame::Initialize()
{
	m_camera.m_AspectRatio = 1;
	m_camera.m_rigidBodyStatue.position = Math::sVector(0, 15, 15);
	const auto rotationZY = Math::cQuaternion(-45.0f / 180 * M_PI, Math::sVector(1, 0, 0));
	//const auto rotationXZ = Math::cQuaternion(M_PI, Math::sVector(0, 1, 0));
	//const auto rotationXZ = Math::cQuaternion(45, Math::sVector(0, 0, 1));
	m_camera.m_rigidBodyStatue.orientation = m_camera.m_rigidBodyStatue.orientation * rotationZY;
	m_camera.m_rigidBodyStatue.orientation.Normalize();

	m_camera.m_FOV = Deg2Rad(45);
	m_camera.m_ZFarPlane = 100.f;
	m_camera.m_ZNearPlane = 0.01f;
	m_isCameraFollow = true;


	eae6320::Graphics::Geometry::cGeometry geometryA("data/geometries/object2.bin");
	eae6320::Graphics::Geometry::cGeometry geometryB("data/geometries/object3.bin");
	eae6320::Graphics::Geometry::cGeometry geometryC("data/geometries/objectCube2.bin");
	auto resultGeometryA = geometryA.Load();
	auto resultGeometryB = geometryB.Load();
	auto resultGeometryC = geometryC.Load();
	eae6320::Graphics::cEffect effectA("data/effects/effectA.bin");
	eae6320::Graphics::cEffect effectB("data/effects/effectB.bin");
	eae6320::Graphics::cEffect effectC("data/effects/effectC.bin");

	auto resultEffectA = effectA.Load();
	auto resultEffectB = effectB.Load();
	auto resultEffectC = effectC.Load();
	m_effectChangeA = effectA;
	m_effectChangeB = effectB;
	m_effectChangeC = effectC;
	std::vector<Application::GameObject> objs;

	if (resultGeometryA && resultEffectB)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryA, effectB)));
	}
	if (resultGeometryC && resultEffectB)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryC, effectB)));
	}
	if (resultGeometryB && resultEffectB)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryB, effectB)));
	}

	SetGameObjects(objs);
	
	return Results::Success;
}


PlutoShe::Physics::Polythedron GetPolythedronFromGameObject(eae6320::Application::GameObject &i_b)
{
	std::vector<PlutoShe::Physics::Vector3> vs;
	auto *renderobject = eae6320::Graphics::Geometry::cGeometryRenderTarget::s_manager.Get(i_b.m_renderObject.m_geometry.m_handler);
	eae6320::Math::sVector sv;
	for (size_t i = 0; i < renderobject->m_vertices.size(); i++)
	{
		sv.x = renderobject->m_vertices[i].m_x;
		sv.y = renderobject->m_vertices[i].m_y;
		sv.z = renderobject->m_vertices[i].m_z;
		sv = i_b.m_renderObject.m_Transformation * sv;
		vs.push_back(PlutoShe::Physics::Vector3(sv.x, sv.y, sv.z));
	}
	return PlutoShe::Physics::Polythedron(vs);
}

void eae6320::cMyGame::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
	isJump -= i_elapsedSecondCount_sinceLastUpdate;
}

void eae6320::cMyGame::UpdateSimulationBasedOnInput()
{
	
	
	// Object movement
	// camera Movement;
	Math::sVector objectVelocity(0, 0, 0);
	Math::sVector objectAcceleration(0, 0, 0);
	Math::sVector cameraVelocity(0, 0, 0), cameraAngularVelocity(0, 0, 0), cameraPolarVelocity(0, 0, 0);
	auto rotationZY = Math::cQuaternion(45.0f / 180 * M_PI, Math::sVector(1, 0, 0));
	auto go = m_camera.m_rigidBodyStatue.orientation * rotationZY;
	float objectSpeed = 2.f;
	auto playerXZ = Math::cMatrix_transformation(go, m_camera.m_rigidBodyStatue.position);


	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Space))
	{
		isJump = 0.3f;
	}
	else
	{
		if (isJump > 0)
		{
			objectAcceleration.y = 10;
		}
	}


	if (UserInput::IsKeyPressed(UserInput::KeyCodes::W))
	{
		objectVelocity -= objectSpeed * playerXZ.GetBackDirection();
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::S))
	{
		objectVelocity += objectSpeed * playerXZ.GetBackDirection();

	}
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::A))
	{
		objectVelocity -= objectSpeed * playerXZ.GetRightDirection();
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::D))
	{
		objectVelocity += objectSpeed * playerXZ.GetRightDirection();
	}
	if (m_gameObjects.size() > 0)
	{
		m_gameObjects[0].m_rigidBodyStatue.velocity.x = objectVelocity.x;
		m_gameObjects[0].m_rigidBodyStatue.velocity.z = objectVelocity.z;
		m_gameObjects[0].m_rigidBodyStatue.acceleration = objectAcceleration;
	}

	float cameraSpeed = 5.f;
	float cameraAngularSpeed = 1.f;
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Up))
	{
		cameraVelocity.z += cameraSpeed;
	}
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Down))
	{
		cameraVelocity.z -= cameraSpeed;
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Left))
	{
		cameraVelocity.x -= cameraSpeed;
	}
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Right))
	{
		cameraVelocity.x += cameraSpeed;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Q))
	{
		//cameraAngularVelocity.y += cameraAngularSpeed;
		cameraPolarVelocity.y += 60.0f  / 180 * M_PI;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::E))
	{
		//cameraAngularVelocity.y -= cameraAngularSpeed;
		cameraPolarVelocity.y -= 60.0f / 180 * M_PI;
	}
	//m_camera.SetVelocityInCameraAxis(cameraVelocity);

	if (m_isCameraFollow)
	{
		m_camera.m_rigidBodyStatue.polarOrigin = m_gameObjects[0].m_rigidBodyStatue.position;
		m_camera.m_rigidBodyStatue.velocity = objectVelocity;
		m_camera.m_rigidBodyStatue.acceleration = objectAcceleration;
	}

	m_camera.SetAngularVelocity(cameraAngularVelocity);
	m_camera.SetPolarVelocity(cameraPolarVelocity);
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
		/*m_isJPressed = false;
		eae6320::Graphics::Geometry::cGeometry geometryB;
		geometryB.m_path = "data/geometries/object2.bin";
		geometryB.Load();*/

		/*eae6320::Graphics::Effect effectB;
		effectB->SetVertexShaderPath("data/shaders/vertex/standard.shader");
		effectB->SetFragmentShaderPath("data/shaders/fragment/vertexColor.shader");*/
		
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
	if (m_gameObjects.size() > 0) 
	{
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::K))
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeA;
		}
		else
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeB;
			
		}
	}
	if (m_gameObjects.size() >= 3)
	{
		if (PlutoShe::Physics::IsCollided(GetPolythedronFromGameObject(m_gameObjects[0]), GetPolythedronFromGameObject(m_gameObjects[2])))
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeA;
			Logging::OutputMessage("collision detected!");
		}
		if (PlutoShe::Physics::IsCollided(GetPolythedronFromGameObject(m_gameObjects[0]), GetPolythedronFromGameObject(m_gameObjects[1])))
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeC;
			Logging::OutputMessage("collision detected!");
		}
	}
}

// Initialization / Clean Up
//--------------------------



eae6320::cResult eae6320::cMyGame::CleanUp()
{
	
	Application::cbApplication::CleanUp();
	return Results::Success;
}
