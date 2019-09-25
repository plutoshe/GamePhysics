#include "GameObject.h"

void eae6320::Application::GameObject::Update(const float i_deltaTime)
{
	m_rigidBodyStatue.Update(i_deltaTime);
	m_renderObject.m_Transformation = eae6320::Math::cMatrix_transformation(m_rigidBodyStatue.orientation, m_rigidBodyStatue.position);
}
