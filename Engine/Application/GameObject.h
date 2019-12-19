#pragma once
#include <Engine/Graphics/cRenderObject.h>
#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/PhysicsSystem/PhysicsSystem.h>
namespace eae6320
{
	namespace Application
	{
		class GameObject
		{
		public:
			eae6320::Graphics::RenderObject m_renderObject;
			eae6320::Physics::sRigidBodyState m_rigidBodyState;
			bool m_isVisiable;
			GameObject() : m_renderObject(), m_rigidBodyState(), m_isVisiable(true) {}
			GameObject(eae6320::Graphics::RenderObject i_renderObject) :m_renderObject(i_renderObject), m_rigidBodyState(), m_isVisiable(true) {}
			GameObject(eae6320::Graphics::RenderObject i_renderObject, eae6320::Physics::sRigidBodyState i_rigidbodyState) :m_renderObject(i_renderObject), m_rigidBodyState(i_rigidbodyState), m_isVisiable(true) {}
		};

	}
}