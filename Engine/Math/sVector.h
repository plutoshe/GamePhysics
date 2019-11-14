/*
	This struct represents a position or direction
*/

#ifndef EAE6320_MATH_SVECTOR_H
#define EAE6320_MATH_SVECTOR_H
#include <cmath>
#define M_PI 3.1415926f
// Struct Declaration
//===================

namespace eae6320
{
	namespace Math
	{
		struct sVector
		{
			// Data
			//=====

			float x = 0.0f, y = 0.0f, z = 0.0f;

			// Interface
			//==========

			// Addition
			//---------

			sVector operator +( const sVector i_rhs ) const;
			sVector& operator +=( const sVector i_rhs );

			sVector operator +( const float i_rhs ) const;
			sVector& operator +=( const float i_rhs );
			friend sVector operator +( const float i_lhs, const sVector i_rhs );


			// Subtraction / Negation
			//-----------------------

			sVector operator -( const sVector i_rhs ) const;
			sVector& operator -=( const sVector i_rhs );
			sVector operator -() const;

			sVector operator -( const float i_rhs ) const;
			sVector& operator -=( const float i_rhs );
			friend sVector operator -( const float i_lhs, const sVector i_rhs );

			// Multiplication
			//---------------

			sVector operator *( const float i_rhs ) const;
			sVector& operator *=( const float i_rhs );
			friend sVector operator *( const float i_lhs, const sVector i_rhs );

			// Division
			//---------

			sVector operator /( const float i_rhs ) const;
			sVector& operator /=( const float i_rhs );

			// Length / Normalization
			//-----------------------

			float GetLength() const;
			float Normalize();
			sVector GetNormalized() const;

			// Products
			//---------

			friend float Dot( const sVector i_lhs, const sVector i_rhs );
			friend sVector Cross( const sVector i_lhs, const sVector i_rhs );

			// Comparison
			//-----------

			bool operator ==( const sVector i_rhs ) const;
			bool operator !=( const sVector i_rhs ) const;

			// Initialization / Clean Up
			//--------------------------

			sVector() = default;
			sVector( const float i_x, const float i_y, const float i_z );
			sVector CartesianToPolarCoordinate() {
				auto l = GetLength();
				auto sy = z == 0 ? 0 : atan(x / z);

				return sVector(l, z < 0? sy + M_PI : sy, l == 0 ? 0 : asin(y / l));
			}
			sVector PolarTocartesianCoordinate()
			{
				return sVector(x * cos(z) * sin(y), x * sin(z), x * cos(z) * cos(y));
			}
		};

		// Friends
		//========

		sVector operator +( const float i_lhs, const sVector i_rhs );
		sVector operator -( const float i_lhs, const sVector i_rhs );
		sVector operator *( const float i_lhs, const sVector i_rhs );
		float Dot( const sVector i_lhs, const sVector i_rhs );
		sVector Cross( const sVector i_lhs, const sVector i_rhs );
	}
}

#endif	// EAE6320_MATH_SVECTOR_H
