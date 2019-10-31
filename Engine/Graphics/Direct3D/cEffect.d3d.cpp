#include "../cEffect.h"
#include "../GraphicsEnv.h"

eae6320::cResult eae6320::Graphics::cEffect::Prepare(
	eae6320::Assets::cManager<eae6320::Graphics::cShader>& manager)
{
	auto result = eae6320::Results::Success;
	if (!(result = PrepareShaderData()))
	{
		EAE6320_ASSERTF(false, "Can't initialize shader for effect");
		return result;
	}
	return result;
}

eae6320::cResult eae6320::Graphics::EffectAsset::Release()
{
	return Results::Success;
}