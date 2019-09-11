#include "Graphics.h"
#include "GraphicsEnv.h"
#include <vector>

namespace eae6320
{
	namespace Graphics
	{
		void ChangeBackgroundColor(std::vector<float> updateColor)
		{
			eae6320::Graphics::Env::s_BackgroundColor = updateColor;
		}
	}
}