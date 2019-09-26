#pragma once
#include "PlatformIncludes.h"
#include "sContext.h"
#include "cConstantBuffer.h"
#include "ConstantBufferFormats.h"
#include "cEffect.h"
#include "cGeometry.h"
#include "cRenderObject.h"
#include "cRenderState.h"
#include "cVertexFormat.h"

#include <Engine/Concurrency/cEvent.h>
namespace eae6320
{
	namespace Graphics
	{
		namespace Env
		{
#if defined( EAE6320_PLATFORM_D3D )
			// In Direct3D "views" are objects that allow a texture to be used a particular way:
			// A render target view allows a texture to have color rendered to it
			extern ID3D11RenderTargetView* s_renderTargetView;
			// A depth/stencil view allows a texture to have depth rendered to it
			extern ID3D11DepthStencilView* s_depthStencilView;
#elif defined( EAE6320_PLATFORM_GL )	

#endif
			// Constant buffer object
			extern eae6320::Graphics::cConstantBuffer s_constantBuffer_frame;
			extern eae6320::Graphics::cConstantBuffer s_constantBuffer_drawCall;

			// Submission Data
			//----------------

			// This struct's data is populated at submission time;
			// it must cache whatever is necessary in order to render a frame
			struct sDataRequiredToRenderAFrame
			{
				eae6320::Graphics::ConstantBufferFormats::sFrame constantData_frame;
				std::vector<float> m_backgroundColor;
				std::vector<eae6320::Graphics::RenderObject> m_renderObjects;
			};
			// In our class there will be two copies of the data required to render a frame:
			//	* One of them will be in the process of being populated by the data currently being submitted by the application loop thread
			//	* One of them will be fully populated and in the process of being rendered from in the render thread
			// (In other words, one is being produced while the other is being consumed)
			extern sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
			extern sDataRequiredToRenderAFrame* s_dataBeingSubmittedByApplicationThread;
			extern sDataRequiredToRenderAFrame* s_dataBeingRenderedByRenderThread;
			extern std::vector<eae6320::Graphics::RenderObject> s_waitingClearRenderObjects;
			// The following two events work together to make sure that
			// the main/render thread and the application loop thread can work in parallel but stay in sync:
			// This event is signaled by the application loop thread when it has finished submitting render data for a frame
			// (the main/render thread waits for the signal)
			extern eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
			// This event is signaled by the main/render thread when it has swapped render data pointers.
			// This means that the renderer is now working with all the submitted data it needs to render the next frame,
			// and the application loop thread can start submitting data for the following frame
			// (the application loop thread waits for the signal)
			extern eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;

			// Shading Data
			//-------------
			extern std::map<std::string, eae6320::Graphics::cShader::Handle> s_vertexShaders;
			extern std::map<std::string, eae6320::Graphics::cShader::Handle> s_fragmentShaders;


			extern eae6320::Graphics::cRenderState::Handle s_renderState;
			// Geometry Data
			//--------------
/*
			extern std::vector<eae6320::Graphics::Effect> s_effects;
			extern std::vector<eae6320::Graphics::Geometry::cGeometryRenderTarget> s_geometries;*/
			extern eae6320::Graphics::cVertexFormat::Handle s_vertexFormat;
			extern std::vector<float> s_BackgroundColor;

		}
	}
}