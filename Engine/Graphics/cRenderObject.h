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
			Graphics::Effect *m_effect;
			eae6320::Math::cMatrix_transformation m_Transformation;

			RenderObject(Graphics::Geometry::cGeometry i_geometry, Graphics::Effect *i_effect) 
			{
				m_geometry = i_geometry;
				m_effect = nullptr;
				if (i_effect != nullptr)
				{
					i_effect->SetToPointer(m_effect);
				}
			}
			RenderObject(const RenderObject &c)
			{
				m_geometry = c.m_geometry;
				m_effect = nullptr;
				if (c.m_effect != nullptr)
				{
					c.m_effect->SetToPointer(m_effect);
				}
				m_Transformation = c.m_Transformation;
			}
			RenderObject() {m_effect = nullptr; }
			~RenderObject() { 
				if (m_effect != nullptr)
				{
					m_effect->DecrementReferenceCount();
				}
			}
		};
	}
}