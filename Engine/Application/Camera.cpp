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


eae6320::Math::cMatrix_transformation eae6320::Application::Camera::GetWorldToCameraForPrediction(float i_predictTime)
{
	return eae6320::Math::cMatrix_transformation::CreateWorldToCameraTransform(m_rigidBodyStatue.PredictFutureOrientation(i_predictTime), m_rigidBodyStatue.PredictFuturePosition(i_predictTime));
}

void eae6320::Application::Camera::SetVelocityInCameraAxis(eae6320::Math::sVector i_VelocityCameraAxis)
{
	auto tranform = eae6320::Math::cMatrix_transformation(m_rigidBodyStatue.orientation, m_rigidBodyStatue.position);
	
	m_rigidBodyStatue.velocity = 
		tranform.GetRightDirection() * i_VelocityCameraAxis.x +
		tranform.GetUpDirection() * i_VelocityCameraAxis.y -
		tranform.GetBackDirection() * i_VelocityCameraAxis.z;
}

void eae6320::Application::Camera::SetAngularVelocity(eae6320::Math::sVector i_angularVelocity)
{
	m_rigidBodyStatue.angularSpeed = i_angularVelocity.y;
}