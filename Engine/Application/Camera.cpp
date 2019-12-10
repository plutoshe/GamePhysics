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
	return eae6320::Math::cMatrix_transformation::CreateWorldToCameraTransform(m_rigidBodyState.orientation, m_rigidBodyState.position);
}


eae6320::Math::cMatrix_transformation eae6320::Application::Camera::GetWorldToCameraForPrediction(float i_predictTime)
{
	return eae6320::Math::cMatrix_transformation::CreateWorldToCameraTransform(m_rigidBodyState.PredictFutureOrientation(i_predictTime), m_rigidBodyState.PredictFuturePosition(i_predictTime));
}

void eae6320::Application::Camera::SetVelocityInCameraAxis(eae6320::Math::sVector i_VelocityCameraAxis)
{
	auto tranform = eae6320::Math::cMatrix_transformation(m_rigidBodyState.orientation, m_rigidBodyState.position);
	
	m_rigidBodyState.velocity = 
		tranform.GetRightDirection() * i_VelocityCameraAxis.x +
		tranform.GetUpDirection() * i_VelocityCameraAxis.y -
		tranform.GetBackDirection() * i_VelocityCameraAxis.z;
}