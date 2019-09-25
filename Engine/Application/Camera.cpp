#include "Camera.h"

eae6320::Math::cMatrix_transformation eae6320::Application::Camera::GetProjectionMatrix()
{
	return eae6320::Math::cMatrix_transformation::CreateCameraToProjectedTransform_perspective(
		m_FOV,
		m_AspectRatio,
		m_ZNearPlane,
		m_ZFarPlane);
}

eae6320::Math::cMatrix_transformation eae6320::Application::Camera::GetWorldToCamera()
{
	return eae6320::Math::cMatrix_transformation::CreateWorldToCameraTransform(m_rigidBodyStatue.orientation, m_rigidBodyStatue.position);
}