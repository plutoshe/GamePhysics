#pragma once
#include "PlatformIncludes.h"
#include "GraphicsEnv.h"
namespace eae6320
{
	namespace Graphics
	{
		namespace Env
		{
#if defined( EAE6320_PLATFORM_D3D )
			ID3D11RenderTargetView* s_renderTargetView = nullptr;
			ID3D11DepthStencilView* s_depthStencilView = nullptr;
#elif defined( EAE6320_PLATFORM_GL )
#endif
			eae6320::Graphics::cConstantBuffer s_constantBuffer_frame(eae6320::Graphics::ConstantBufferTypes::Frame);
			eae6320::Graphics::cConstantBuffer s_constantBuffer_drawCall(eae6320::Graphics::ConstantBufferTypes::DrawCall);
			sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
			sDataRequiredToRenderAFrame* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
			sDataRequiredToRenderAFrame* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
			std::vector<eae6320::Graphics::RenderObject> s_waitingClearRenderObjects;
			eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
			eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;
			std::map<std::string, eae6320::Graphics::cShader::Handle> s_vertexShaders;
			std::map<std::string, eae6320::Graphics::cShader::Handle> s_fragmentShaders;
			eae6320::Graphics::cVertexFormat::Handle s_vertexFormat;
			eae6320::Graphics::cRenderState::Handle s_renderState; 
			std::vector<float> s_BackgroundColor{ 1.0f, 0.0f, 1.0f, 1.0f };
			/*std::vector<eae6320::Graphics::Effect> s_effects;
			std::vector<eae6320::Graphics::Geometry::cGeometryRenderTarget> s_geometries;*/
			
			
		}
	}
}