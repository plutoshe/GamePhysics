#pragma once
#include <Engine/Graphics/cRenderObject.h>
#include <Engine/Physics/sRigidBodyState.h>
namespace eae6320
{
	namespace Application
	{
		class GameObject
		{
		public:
			eae6320::Graphics::RenderObject m_renderObject;
			eae6320::Physics::sRigidBodyState m_rigidBodyStatue;
			bool m_isVisiable;
			void Update(const float i_deltaTime);
			GameObject() {}
			GameObject(eae6320::Graphics::RenderObject i_renderObject) :m_renderObject(i_renderObject) {}
		};

	}
}