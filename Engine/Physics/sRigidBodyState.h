/*
	This struct can be used to represent the state of a
	rigid body in 3D space
*/

#ifndef EAE6320_PHYSICS_SRIGIDBODYSTATE_H
#define EAE6320_PHYSICS_SRIGIDBODYSTATE_H

// Includes
//=========

#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>
#include <Engine/PhysicsSystem/PhysicsSystem.h>
#include <set>
// Forward Declarations
//=====================

namespace eae6320
{
	namespace Math
	{
		class cMatrix_transformation;
	}
}

// Struct Declaration
//===================

namespace eae6320
{
	namespace Physics
	{
		struct sRigidBodyState;

		struct contactInfo {
			Math::sVector contactPointA, contactPointB;
			sRigidBodyState* sa, * sb;
		};

		struct sRigidBodyState
		{
			// Data
			//=====
			static std::set<sRigidBodyState*> s_physicsObjs;
			static void UpdatePhysics();

			bool isStatic = false;
			void EnablePhysicsSimulation() { s_physicsObjs.insert(this); }
			void DisablePhysicsSimulation() { s_physicsObjs.erase(this); }
			
			Math::sVector position;	// In arbitrary units determined by the applicaton's convention
			Math::sVector velocity;	// Distance per-second
			Math::sVector acceleration;	// Distance per-second^2
			Math::sVector polarVelocity;
			Math::sVector polarAcceleration;
			Math::sVector polarOrigin;
			Math::cQuaternion orientation;
			Math::sVector angularVelocity_axis_localXY = Math::sVector( 0.0f, 0.0f, 1.0f );	// In local space (not world space)
			Math::sVector angularVelocity_axis_localXZ = Math::sVector(0.0f, 1.0f, 0.0f);	// In local space (not world space)
			Math::sVector angularVelocity_axis_localYZ = Math::sVector(1.0f, 0.0f, 0.0f);	// In local space (not world space)
			///float angularSpeed = 0.0f;	// Radians per-second (positive values rotate right-handed, negative rotate left-handed)

			Math::sVector angularVelocity;

			Math::sVector forceAccumulator;
			Math::sVector torqueAccumulator;

			float inverseMass = 1;
			Math::cMatrix_transformation inverseInertia = Math::cMatrix_transformation(2.5f, 0, 0, 0, 0, 2.5f, 0, 0, 0, 0, 2.5f, 0, 0, 0, 0, 2.5f);

			Math::sVector linearMomentum;
			Math::sVector angularMomentum;
			Math::sVector localCenter;

			PlutoShe::Physics::ColliderList colliders;

			// Interface
			//==========

			void Update( const float i_secondCountToIntegrate );

			Math::cQuaternion GetCurrentRotation(const float dt) const;
			Math::sVector PredictFuturePosition( const float i_secondCountToExtrapolate ) const;
			Math::cQuaternion PredictFutureOrientation( const float i_secondCountToExtrapolate ) const;
			Math::cMatrix_transformation PredictFutureTransform( const float i_secondCountToExtrapolate ) const;
			
			void ApplyForce(const Math::sVector& f, const Math::sVector& atPosition);
			void AddCollider(PlutoShe::Physics::Collider i_collider);

			sRigidBodyState() : colliders() {}
			sRigidBodyState(PlutoShe::Physics::Collider i_colliders) { 
				colliders = i_colliders; 
				localCenter = colliders.GetCenter().TosVector(); 
			}
			sRigidBodyState(PlutoShe::Physics::ColliderList i_colliders) { colliders = i_colliders; localCenter = colliders.GetCenter().TosVector();}

			Math::sVector WorldCenter() { return position + orientation * localCenter; }

			bool isCollide(sRigidBodyState& i_b) { return colliders.IsCollided(i_b.colliders); }
		};
	}
}

#endif	// EAE6320_PHYSICS_SRIGIDBODYSTATE_H
