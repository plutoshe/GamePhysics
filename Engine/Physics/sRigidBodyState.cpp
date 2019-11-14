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
	{
		position += velocity * i_secondCountToIntegrate;
	}
	// Update velocity
	{
		velocity += acceleration * i_secondCountToIntegrate;
	}
	// update position based on polar velocity
	if (polarVelocity != Math::sVector(0,0,0))
	{
		Math::sVector poloarConversion = position - polarOrigin;
		//poloarConversion.z = position.z + polarOrigin.z;
		Math::sVector polarPosition = poloarConversion.CartesianToPolarCoordinate();

		polarPosition += polarVelocity * i_secondCountToIntegrate;
		position = polarPosition.PolarTocartesianCoordinate() + polarOrigin;
		const auto rotationXZ = Math::cQuaternion(polarVelocity.y * i_secondCountToIntegrate, Math::sVector(0, 1, 0));
		orientation = rotationXZ * orientation;
		orientation.Normalize();
	}
	// update polar velocity
	{
		polarVelocity += polarAcceleration;
	}
	
	// Update orientation
	{
		const auto rotation = Math::cQuaternion(angularSpeed * i_secondCountToIntegrate, angularVelocity_axis_local);
		orientation = orientation * rotation;
		orientation.Normalize();
	}
}

eae6320::Math::sVector eae6320::Physics::sRigidBodyState::PredictFuturePosition( const float i_secondCountToExtrapolate ) const
{
	return position + ( velocity * i_secondCountToExtrapolate );
}

eae6320::Math::cQuaternion eae6320::Physics::sRigidBodyState::PredictFutureOrientation( const float i_secondCountToExtrapolate ) const
{
	const auto rotation = Math::cQuaternion( angularSpeed * i_secondCountToExtrapolate, angularVelocity_axis_local );
	return Math::cQuaternion( orientation * rotation ).GetNormalized();
}

eae6320::Math::cMatrix_transformation eae6320::Physics::sRigidBodyState::PredictFutureTransform( const float i_secondCountToExtrapolate ) const
{
	return Math::cMatrix_transformation( PredictFutureOrientation( i_secondCountToExtrapolate ), PredictFuturePosition( i_secondCountToExtrapolate ) );
}
