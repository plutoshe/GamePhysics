#pragma once
#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Math/cMatrix_transformation.h>

namespace eae6320
{
	namespace Application
	{
		class Camera
		{
		public:
			Camera() {}
			eae6320::Physics::sRigidBodyState m_rigidBodyStatue;
			float m_ZNearPlane;
			float m_ZFarPlane;
			float m_AspectRatio;
			float m_FOV;
			eae6320::Math::cMatrix_transformation GetProjectionMatrix();
			//eae6320::Math::cMatrix_transformation GetCameraToWorld();
			eae6320::Math::cMatrix_transformation GetWorldToCamera();
		};
	}
}

