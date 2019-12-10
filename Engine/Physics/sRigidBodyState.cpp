// Includes
//=========

#include "sRigidBodyState.h"
#include <cmath>
#include <Engine/Math/cMatrix_transformation.h>

// Interface
//==========

void eae6320::Physics::sRigidBodyState::Update( const float i_secondCountToIntegrate )
{
	// Update position
	//{
	//	position += velocity * i_secondCountToIntegrate;
	//}
	//// Update velocity
	//{
	//	velocity += acceleration * i_secondCountToIntegrate;
	//}
	//// update position based on polar velocity
	//if (polarVelocity != Math::sVector(0,0,0))
	//{
	//	Math::sVector poloarConversion = position - polarOrigin;
	//	//poloarConversion.z = position.z + polarOrigin.z;
	//	Math::sVector polarPosition = poloarConversion.CartesianToPolarCoordinate();

	//	polarPosition += polarVelocity * i_secondCountToIntegrate;
	//	position = polarPosition.PolarTocartesianCoordinate() + polarOrigin;
	//	const auto rotationXZ = Math::cQuaternion(polarVelocity.y * i_secondCountToIntegrate, Math::sVector(0, 1, 0));
	//	orientation = rotationXZ * orientation;
	//	orientation.Normalize();
	//}
	//// update polar velocity
	//{
	//	polarVelocity += polarAcceleration;
	//}
	//
	//// Update orientation
	//{
	//	const auto rotation = Math::cQuaternion(angularSpeed * i_secondCountToIntegrate, angularVelocity_axis_local);
	//	orientation = orientation * rotation;
	//	orientation.Normalize();
	//}

	auto dt = i_secondCountToIntegrate;

	auto linearMomentumDelta = dt * forceAccumulator;
	auto velocityDelta = linearMomentumDelta * inverseMass;
	
	linearMomentum += linearMomentum;
	velocity += velocityDelta;
	position += velocity * dt;
	
	auto angularMomentumDelta = dt * torqueAccumulator;
	
	auto invertOrientation = orientation;
	invertOrientation.Invert();
	auto angularVelocityDelta = 
		Math::cMatrix_transformation(orientation, Math::sVector(0, 0, 0)) * 
		inverseInertia * Math::cMatrix_transformation(invertOrientation, Math::sVector(0, 0, 0)) *
		angularMomentumDelta;
	angularVelocity += angularVelocityDelta;
	orientation = orientation * GetCurrentRotation(dt);

	forceAccumulator = Math::sVector(0, 0, 0);
	torqueAccumulator = Math::sVector(0, 0, 0);
}

eae6320::Math::sVector eae6320::Physics::sRigidBodyState::PredictFuturePosition( const float i_secondCountToExtrapolate ) const
{
	return position + ( velocity * i_secondCountToExtrapolate );
}

eae6320::Math::cQuaternion eae6320::Physics::sRigidBodyState::GetCurrentRotation(const float dt) const
{
	return Math::cQuaternion(angularVelocity.x * dt, angularVelocity_axis_localXY) *
		Math::cQuaternion(angularVelocity.y * dt, angularVelocity_axis_localXZ) *
		Math::cQuaternion(angularVelocity.z * dt, angularVelocity_axis_localYZ);
		
}

eae6320::Math::cQuaternion eae6320::Physics::sRigidBodyState::PredictFutureOrientation( const float i_secondCountToExtrapolate ) const
{		
	return Math::cQuaternion( orientation * GetCurrentRotation(i_secondCountToExtrapolate)).GetNormalized();
}

eae6320::Math::cMatrix_transformation eae6320::Physics::sRigidBodyState::PredictFutureTransform( const float i_secondCountToExtrapolate ) const
{
	return Math::cMatrix_transformation( PredictFutureOrientation( i_secondCountToExtrapolate ), PredictFuturePosition( i_secondCountToExtrapolate ) );
}

void eae6320::Physics::sRigidBodyState::ApplyForce(const Math::sVector& f, const Math::sVector& atPosition) // local space attach point
{
	forceAccumulator += f;
	auto torque = PlutoShe::Physics::Vector3(atPosition - localCenter).cross(f);
	torqueAccumulator += Math::sVector(torque.m_x, torque.m_y, torque.m_z);
}

void eae6320::Physics::sRigidBodyState::AddCollider(PlutoShe::Physics::Collider i_collider)
{
	colliders.AddCollider(i_collider);
	localCenter = colliders.GetCenter().TosVector();
}