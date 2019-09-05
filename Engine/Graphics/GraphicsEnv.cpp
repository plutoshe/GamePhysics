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
			eae6320::Graphics::cConstantBuffer s_constantBuffer_frame(eae6320::Graphics::ConstantBufferTypes::Frame);
			sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
			sDataRequiredToRenderAFrame* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
			sDataRequiredToRenderAFrame* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
			eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
			eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;
			eae6320::Graphics::cShader::Handle s_vertexShader;
			eae6320::Graphics::cShader::Handle s_fragmentShader;
			eae6320::Graphics::cRenderState::Handle s_renderState;
			eae6320::Graphics::cVertexFormat::Handle s_vertexFormat;
			eae6320::Graphics::Effect s_effect;
			eae6320::Graphics::Geometry::cGeometryRenderTarget s_geometry;
			ID3D11Buffer* s_vertexBuffer;
#elif defined( EAE6320_PLATFORM_GL )

			eae6320::Graphics::cConstantBuffer s_constantBuffer_frame(eae6320::Graphics::ConstantBufferTypes::Frame);
			sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
			sDataRequiredToRenderAFrame* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
			sDataRequiredToRenderAFrame* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
			eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
			eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;
			eae6320::Graphics::cShader::Handle s_vertexShader;
			eae6320::Graphics::cShader::Handle s_fragmentShader;
			GLuint s_programId = 0;
			eae6320::Graphics::cRenderState::Handle s_renderState;
			GLuint s_vertexBufferId = 0;
			GLuint s_vertexArrayId = 0;
			eae6320::Graphics::Effect s_effect;
			eae6320::Graphics::Geometry::cGeometryRenderTarget s_geometry;
#endif

		}
	}
}