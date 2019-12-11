// Includes
//=========

#include "cMyGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/Graphics/cRenderObject.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cGeometry.h>
#include <vector>
#include <Engine/Math/sVector.h>
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
	m_camera.m_rigidBodyState.position = Math::sVector(0, 15, 15);
	const auto rotationZY = Math::cQuaternion(-45.0f / 180 * M_PI, Math::sVector(1, 0, 0));
	//const auto rotationXZ = Math::cQuaternion(M_PI, Math::sVector(0, 1, 0));
	//const auto rotationXZ = Math::cQuaternion(45, Math::sVector(0, 0, 1));
	m_camera.m_rigidBodyState.orientation = m_camera.m_rigidBodyState.orientation * rotationZY;
	m_camera.m_rigidBodyState.orientation.Normalize();

	m_camera.m_FOV = Deg2Rad(45);
	m_camera.m_ZFarPlane = 100.f;
	m_camera.m_ZNearPlane = 0.01f;
	m_isCameraFollow = true;


	eae6320::Graphics::Geometry::cGeometry geometryA("data/geometries/sphereOrigin.bin");
	eae6320::Graphics::Geometry::cGeometry geometryB("data/geometries/plane.bin");
	eae6320::Graphics::Geometry::cGeometry geometryC("data/geometries/objectCube2.bin");
	auto resultGeometryA = geometryA.Load();
	auto resultGeometryB = geometryB.Load();
	auto resultGeometryC = geometryC.Load();
	eae6320::Graphics::cEffect effectA("data/effects/effectA.bin");
	eae6320::Graphics::cEffect effectB("data/effects/effectB.bin");
	eae6320::Graphics::cEffect effectC("data/effects/effectC.bin");
	PlutoShe::Physics::Collider colliderA("data/colliders/sphereOrigin.bin");
	PlutoShe::Physics::Collider colliderB("data/colliders/plane.bin");
	PlutoShe::Physics::Collider colliderC("data/colliders/objectCube2.bin");

	auto resultEffectA = effectA.Load();
	auto resultEffectB = effectB.Load();
	auto resultEffectC = effectC.Load();
	m_effectChangeA = effectB;
	m_effectChangeB = effectA;
	m_effectChangeC = effectC;
	std::vector<Application::GameObject> objs;

	if (resultGeometryA && resultEffectB)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryA, effectB), eae6320::Physics::sRigidBodyState(colliderA)));

	}
	if (resultGeometryC && resultEffectB)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryC, effectB), eae6320::Physics::sRigidBodyState(colliderC)));
	}
	if (resultGeometryB && resultEffectB)
	{
		objs.push_back(Application::GameObject(Graphics::RenderObject(geometryB, effectB), eae6320::Physics::sRigidBodyState(colliderB)));
	}


	SetGameObjects(objs);
	m_gameObjects[0].m_rigidBodyState.position.x = -2.1f;
	m_gameObjects[0].m_rigidBodyState.EnablePhysicsSimulation();
	m_gameObjects[1].m_rigidBodyState.EnablePhysicsSimulation();
	eae6320::Audio3D::AudioSource* MySource;
	auto result = eae6320::Audio3D::AudioSource::Load("data/audiosources/a.wav", MySource);
	if (!result)
	{
		Logging::OutputError("Load audio failed");
		return result;
	}
	MySource->PlayLooped();
	//for (int i = 0; i < m_gameObjects.size(); i++)
	//{
	//	auto* renderobject = eae6320::Graphics::Geometry::cGeometryRenderTarget::s_manager.Get(m_gameObjects[i].m_renderObject.m_geometry.m_handler);
	//	std::vector<PlutoShe::Physics::Vector3> vertices;
	//	if (renderobject)
	//	{
	//		for (int vi = 0; vi < renderobject->m_vertices.size(); vi++)
	//		{
	//			vertices.push_back(PlutoShe::Physics::Vector3(renderobject->m_vertices[vi].m_x, renderobject->m_vertices[vi].m_y, renderobject->m_vertices[vi].m_z));
	//		}
	//		m_gameObjects[i].m_colliders.AddCollider(PlutoShe::Physics::Collider(vertices));
	//	}
	//}

	return Results::Success;
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
	auto go = m_camera.m_rigidBodyState.orientation * rotationZY;
	float objectSpeed = 2.f;
	auto playerXZ = Math::cMatrix_transformation(go, m_camera.m_rigidBodyState.position);


	//if(UserInput::IsKeyPressed(UserInput::KeyCodes::Space))
	//{
	//	isJump = 0.3f;
	//}
	//else
	//{
	//	if (isJump > 0)
	//	{
	//		objectAcceleration.y = 10;
	//	}
	//}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Space))
	{
		m_gameObjects[0].m_rigidBodyState.ApplyForce(eae6320::Math::sVector(0, 0, -10), m_gameObjects[0].m_rigidBodyState.WorldCenter() + eae6320::Math::sVector(0, 1.414f, 1.414f));
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::W))
	{
		m_gameObjects[0].m_rigidBodyState.ApplyForce(eae6320::Math::sVector(0, 0, -10), m_gameObjects[0].m_rigidBodyState.WorldCenter() + eae6320::Math::sVector(0, 0.7f, 0.7f));
		//objectVelocity -= objectSpeed * playerXZ.GetBackDirection();
		
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::S))
	{
		//objectVelocity += objectSpeed * playerXZ.GetBackDirection();
		m_gameObjects[0].m_rigidBodyState.ApplyForce(eae6320::Math::sVector(0, 0, 10), m_gameObjects[0].m_rigidBodyState.WorldCenter() + eae6320::Math::sVector(0, 0.7f, -0.7f));
	}
	
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::A))
	{
		m_gameObjects[0].m_rigidBodyState.ApplyForce(eae6320::Math::sVector(-10, 0, 0), m_gameObjects[0].m_rigidBodyState.WorldCenter() + eae6320::Math::sVector(0.7f, 0.7f, 0));
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::D))
	{
		m_gameObjects[0].m_rigidBodyState.ApplyForce(eae6320::Math::sVector(10, 0, 0), m_gameObjects[0].m_rigidBodyState.WorldCenter() + eae6320::Math::sVector(-0.7f, 0.7f, 0));
	}
	if (m_gameObjects.size() > 0)
	{
		//m_gameObjects[0].m_rigidBodyState.velocity.x = objectVelocity.x;
		//m_gameObjects[0].m_rigidBodyState.velocity.z = objectVelocity.z;
		//m_gameObjects[0].m_rigidBodyState.acceleration = objectAcceleration;
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
		m_camera.m_rigidBodyState.polarOrigin = m_gameObjects[0].m_rigidBodyState.position;
		m_camera.m_rigidBodyState.velocity = objectVelocity;
		m_camera.m_rigidBodyState.acceleration = objectAcceleration;
	}

	//m_camera.SetAngularVelocity(cameraAngularVelocity);
	//m_camera.SetPolarVelocity(cameraPolarVelocity);
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
	/*if (m_gameObjects.size() > 0) 
	{
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::K))
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeA;
		}
		else
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeB;
			
		}
	}*/
	if (m_gameObjects.size() >= 3)
	{
		if (m_gameObjects[0].m_rigidBodyState.colliders.IsCollided(m_gameObjects[1].m_rigidBodyState.colliders))
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeB;
		}
		else
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeA;
		}
		/*if (m_gameObjects[0].m_rigidBodyState.colliders.IsCollided(m_gameObjects[1].m_rigidBodyState.colliders))
		{
			m_gameObjects[0].m_renderObject.m_effect = m_effectChangeC;
		}*/
	}
}

// Initialization / Clean Up
//--------------------------



eae6320::cResult eae6320::cMyGame::CleanUp()
{
	
	Application::cbApplication::CleanUp();
	return Results::Success;
}
