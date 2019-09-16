#pragma once
#include "cGeometry.h"
#include "cEffect.h"

namespace eae6320
{
	namespace Graphics
	{
		struct RenderObject
		{
			Graphics::Geometry::cGeometryRenderTarget *m_geometry;
			Graphics::Effect *m_effect;
			RenderObject(Graphics::Geometry::cGeometryRenderTarget *i_geometry, Graphics::Effect *i_effect) 
			{
				m_geometry = nullptr;
				m_effect = nullptr;
				if (i_geometry != nullptr)
				{
					i_geometry->SetToPointer(m_geometry);
				}
				if (i_effect != nullptr)
				{
					i_effect->SetToPointer(m_effect);
				}
			}
			RenderObject(const RenderObject &c)
			{
				m_geometry = nullptr;
				m_effect = nullptr;
				if (c.m_geometry != nullptr)
				{
					c.m_geometry->SetToPointer(m_geometry);
				}
				if (c.m_effect != nullptr)
				{
					c.m_effect->SetToPointer(m_effect);
				}
			}
			RenderObject() { m_geometry = nullptr; m_effect = nullptr; }
			~RenderObject() { 
				if (m_geometry != nullptr)
				{
					m_geometry->DecrementReferenceCount();
				}
				if (m_effect != nullptr)
				{
					m_effect->DecrementReferenceCount();
				}
			}
		};
	}
}