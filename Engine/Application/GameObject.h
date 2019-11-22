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
			eae6320::Physics::sRigidBodyState m_rigidBodyStatue;
			PlutoShe::Physics::ColliderList m_colliders;
			bool m_isVisiable;
			GameObject() : m_renderObject(), m_rigidBodyStatue(), m_isVisiable(true), m_colliders() {}
			GameObject(eae6320::Graphics::RenderObject i_renderObject) :m_renderObject(i_renderObject), m_isVisiable(true) {}
			GameObject(eae6320::Graphics::RenderObject i_renderObject, PlutoShe::Physics::ColliderList i_colliderList) :m_renderObject(i_renderObject), m_isVisiable(true), m_colliders(i_colliderList) {}
		};

	}
}