#pragma once
#include <Engine/Math/cMatrix_transformation.h>
#include "cGeometry.h"
#include "cEffect.h"

namespace eae6320
{
	namespace Graphics
	{
		struct RenderObject
		{
			Graphics::Geometry::cGeometry m_geometry;
			Graphics::cEffect m_effect;
			eae6320::Math::cMatrix_transformation m_Transformation;

			RenderObject(Graphics::Geometry::cGeometry i_geometry, Graphics::cEffect i_effect) 
			{
				m_geometry = i_geometry;
				m_effect = i_effect;
			}
			RenderObject(const RenderObject &c)
			{
				m_geometry = c.m_geometry;
				m_effect = c.m_effect;
			}
			RenderObject() { }
			~RenderObject() { 
				
			}
		};
	}
}