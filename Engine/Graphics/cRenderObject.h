#pragma once
#include "cGeometry.h"
#include "cEffect.h"

namespace eae6320
{
	namespace Graphics
	{
		struct RenderObject
		{
			Graphics::Geometry::cGeometryRenderTarget m_geometry;
			Graphics::Effect m_effect;
			RenderObject(Graphics::Geometry::cGeometryRenderTarget i_geometry, Graphics::Effect i_effect) : m_geometry(i_geometry), m_effect(i_effect) {}
			RenderObject() {}
		};
	}
}