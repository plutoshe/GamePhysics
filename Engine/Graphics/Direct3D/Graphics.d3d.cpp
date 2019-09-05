// Includes
//=========

#include "../Graphics.h"

#include "Includes.h"
#include "../cConstantBuffer.h"
#include "../ConstantBufferFormats.h"
#include "../cRenderState.h"
#include "../cShader.h"
#include "../cVertexFormat.h"
#include "../sContext.h"
#include "../cGeometry.h"
#include "../cEffect.h"
#include "../GraphicsEnv.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>
#include <utility>

// Helper Function Declarations
//=============================

namespace
{
	eae6320::cResult InitializeGeometry();
	eae6320::cResult InitializeShadingData();
	eae6320::cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);
}

// Interface
//==========

// Submission
//-----------

void eae6320::Graphics::SubmitElapsedTime(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime)
{
	EAE6320_ASSERT(eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread);
	auto& constantData_frame = eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread->constantData_frame;
	constantData_frame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
	constantData_frame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
}

eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted(const unsigned int i_timeToWait_inMilliseconds)
{
	return Concurrency::WaitForEvent(eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds);
}

eae6320::cResult eae6320::Graphics::SignalThatAllDataForAFrameHasBeenSubmitted()
{
	return eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
}

// Render
//-------

void eae6320::Graphics::RenderFrame()
{
	// Wait for the application loop to submit data to be rendered
	{
		const auto result = Concurrency::WaitForEvent(eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread);
		if (result)
		{
			// Switch the render data pointers so that
			// the data that the application just submitted becomes the data that will now be rendered
			std::swap(eae6320::Graphics::Env::s_dataBeingSubmittedByApplicationThread, eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread);
			// Once the pointers have been swapped the application loop can submit new data
			const auto result = eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal();
			if (!result)
			{
				EAE6320_ASSERTF(false, "Couldn't signal that new graphics data can be submitted");
				Logging::OutputError("Failed to signal that new render data can be submitted");
				UserOutput::Print("The renderer failed to signal to the application that new graphics data can be submitted."
					" The application is probably in a bad state and should be exited");
				return;
			}
		}
		else
		{
			EAE6320_ASSERTF(false, "Waiting for the graphics data to be submitted failed");
			Logging::OutputError("Waiting for the application loop to submit data to be rendered failed");
			UserOutput::Print("The renderer failed to wait for the application to submit data to be rendered."
				" The application is probably in a bad state and should be exited");
			return;
		}
	}

	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		EAE6320_ASSERT(eae6320::Graphics::Env::s_renderTargetView);

		// Black is usually used
		constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		direct3dImmediateContext->ClearRenderTargetView(eae6320::Graphics::Env::s_renderTargetView, clearColor);
	}
	// In addition to the color buffer there is also a hidden image called the "depth buffer"
	// which is used to make it less important which order draw calls are made.
	// It must also be "cleared" every frame just like the visible color buffer.
	{
		EAE6320_ASSERT(eae6320::Graphics::Env::s_depthStencilView);

		constexpr float clearToFarDepth = 1.0f;
		constexpr uint8_t stencilValue = 0;	// Arbitrary if stencil isn't used
		direct3dImmediateContext->ClearDepthStencilView(eae6320::Graphics::Env::s_depthStencilView, D3D11_CLEAR_DEPTH, clearToFarDepth, stencilValue);
	}

	EAE6320_ASSERT(eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread);

	// Update the frame constant buffer
	{
		// Copy the data from the system memory that the application owns to GPU memory
		auto& constantData_frame = eae6320::Graphics::Env::s_dataBeingRenderedByRenderThread->constantData_frame;
		eae6320::Graphics::Env::s_constantBuffer_frame.Update(&constantData_frame);
	}

	// Bind the shading data
	{
		eae6320::Graphics::Env::s_effect.Bind(cShader::s_manager, eae6320::Graphics::Env::s_vertexShader, eae6320::Graphics::Env::s_fragmentShader);

	}
	// Draw the geometry
	{
		eae6320::Graphics::Env::s_geometry.Draw();
	}

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it the contents of the back buffer must be "presented"
	// (or "swapped" with the "front buffer", which is the image that is actually being displayed)
	{
		auto* const swapChain = sContext::g_context.swapChain;
		EAE6320_ASSERT(swapChain);
		constexpr unsigned int swapImmediately = 0;
		constexpr unsigned int presentNextFrame = 0;
		const auto result = swapChain->Present(swapImmediately, presentNextFrame);
		EAE6320_ASSERT(SUCCEEDED(result));
	}

	// Once everything has been drawn the data that was submitted for this frame
	// should be cleaned up and cleared.
	// so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
	{
		// (At this point in the class there isn't anything that needs to be cleaned up)
	}
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
{
	auto result = Results::Success;

	// Initialize the platform-specific context
	if (!(result = sContext::g_context.Initialize(i_initializationParameters)))
	{
		EAE6320_ASSERTF(false, "Can't initialize Graphics without context");
		return result;
	}
	// Initialize the asset managers
	{
		if (!(result = cRenderState::s_manager.Initialize()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the render state manager");
			return result;
		}
		if (!(result = cShader::s_manager.Initialize()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shader manager");
			return result;
		}
		if (!(result = cVertexFormat::s_manager.Initialize()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the vertex format manager");
			return result;
		}
	}
	// Initialize the platform-independent graphics objects
	{
		if (result = eae6320::Graphics::Env::s_constantBuffer_frame.Initialize())
		{
			// There is only a single frame constant buffer that is reused
			// and so it can be bound at initialization time and never unbound
			eae6320::Graphics::Env::s_constantBuffer_frame.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				ShaderTypes::Vertex | ShaderTypes::Fragment);
		}
		else
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without frame constant buffer");
			return result;
		}
	}
	// Initialize the events
	{
		if (!(result = eae6320::Graphics::Env::s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data has been submitted from the application thread");
			return result;
		}
		if (!(result = eae6320::Graphics::Env::s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
			Concurrency::EventState::Signaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data can be submitted from the application thread");
			return result;
		}
	}
	// Initialize the views
	{
		if (!(result = InitializeViews(i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the views");
			return result;
		}
	}
	// Initialize the shading data
	{
		if (!(result = InitializeShadingData()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
			return result;
		}
	}
	// Initialize the geometry
	{
		if (!(result = InitializeGeometry()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			return result;
		}
	}

	return result;
}

eae6320::cResult eae6320::Graphics::CleanUp()
{
	auto result = Results::Success;

	if (eae6320::Graphics::Env::s_renderTargetView)
	{
		eae6320::Graphics::Env::s_renderTargetView->Release();
		eae6320::Graphics::Env::s_renderTargetView = nullptr;
	}
	if (eae6320::Graphics::Env::s_depthStencilView)
	{
		eae6320::Graphics::Env::s_depthStencilView->Release();
		eae6320::Graphics::Env::s_depthStencilView = nullptr;
	}
	if (eae6320::Graphics::Env::s_vertexBuffer)
	{
		eae6320::Graphics::Env::s_vertexBuffer->Release();
		eae6320::Graphics::Env::s_vertexBuffer = nullptr;
	}
	if (eae6320::Graphics::Env::s_vertexFormat)
	{
		const auto result_vertexFormat = cVertexFormat::s_manager.Release(eae6320::Graphics::Env::s_vertexFormat);
		if (!result_vertexFormat)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_vertexFormat;
			}
		}
	}
	if (eae6320::Graphics::Env::s_vertexShader)
	{
		const auto result_vertexShader = cShader::s_manager.Release(eae6320::Graphics::Env::s_vertexShader);
		if (!result_vertexShader)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_vertexShader;
			}
		}
	}
	if (eae6320::Graphics::Env::s_fragmentShader)
	{
		const auto result_fragmentShader = cShader::s_manager.Release(eae6320::Graphics::Env::s_fragmentShader);
		if (!result_fragmentShader)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_fragmentShader;
			}
		}
	}
	if (eae6320::Graphics::Env::s_renderState)
	{
		const auto result_renderState = cRenderState::s_manager.Release(eae6320::Graphics::Env::s_renderState);
		if (!result_renderState)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_renderState;
			}
		}
	}

	{
		const auto result_constantBuffer_frame = eae6320::Graphics::Env::s_constantBuffer_frame.CleanUp();
		if (!result_constantBuffer_frame)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_constantBuffer_frame;
			}
		}
	}

	{
		const auto result_shaderManager = cShader::s_manager.CleanUp();
		if (!result_shaderManager)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_shaderManager;
			}
		}
	}
	{
		const auto result_renderStateManager = cRenderState::s_manager.CleanUp();
		if (!result_renderStateManager)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_renderStateManager;
			}
		}
	}
	{
		const auto result_vertexFormatManager = cVertexFormat::s_manager.CleanUp();
		if (!result_vertexFormatManager)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_vertexFormatManager;
			}
		}
	}

	{
		const auto result_context = sContext::g_context.CleanUp();
		if (!result_context)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_context;
			}
		}
	}

	return result;
}

// Helper Function Definitions
//============================

namespace
{
	eae6320::cResult InitializeGeometry()
	{
		auto result = eae6320::Results::Success;

		auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
		EAE6320_ASSERT(direct3dDevice);

		// Vertex Format
		{
			if (!(result = eae6320::Graphics::cVertexFormat::s_manager.Load(eae6320::Graphics::VertexTypes::_3dObject, eae6320::Graphics::Env::s_vertexFormat,
				"data/shaders/vertex/vertexinputlayout_3dobject.shader")))
			{
				EAE6320_ASSERTF(false, "Can't initialize geometry without vertex format");
				return result;
			}
		}
		// Vertex Buffer
		{
			eae6320::Graphics::Env::s_geometry.LoadData();

			D3D11_BUFFER_DESC bufferDescription{};
			{
				const auto bufferSize = eae6320::Graphics::Env::s_geometry.BufferSize();
				EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(bufferDescription.ByteWidth) * 8)));
				bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
				bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
				bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
				bufferDescription.MiscFlags = 0;
				bufferDescription.StructureByteStride = 0;	// Not used
			}
			D3D11_SUBRESOURCE_DATA initialData{};
			{
				initialData.pSysMem = eae6320::Graphics::Env::s_geometry.GetVertexData();
				// (The other data members are ignored for non-texture buffers)
			}

			const auto d3dResult = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &eae6320::Graphics::Env::s_vertexBuffer);
			if (FAILED(d3dResult))
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, "3D object vertex buffer creation failed (HRESULT %#010x)", d3dResult);
				eae6320::Logging::OutputError("Direct3D failed to create a 3D object vertex buffer (HRESULT %#010x)", d3dResult);
				return result;
			}
		}

		return result;
	}

	eae6320::cResult InitializeShadingData()
	{
		auto result = eae6320::Results::Success;
		eae6320::Graphics::Env::s_effect.SetVertexShaderPath("data/shaders/vertex/standard.shader");
		eae6320::Graphics::Env::s_effect.SetFragmentShaderPath("data/shaders/fragment/change_color.shader");
		if (!(result = eae6320::Graphics::Env::s_effect.Load(eae6320::Graphics::cShader::s_manager, eae6320::Graphics::Env::s_vertexShader, eae6320::Graphics::Env::s_fragmentShader)))
		{
			EAE6320_ASSERTF(false, "Can't initialize effects");
			return result;
		}
		{
			constexpr uint8_t defaultRenderState = 0;
			if (!(result = eae6320::Graphics::cRenderState::s_manager.Load(defaultRenderState, eae6320::Graphics::Env::s_renderState)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
				return result;
			}
		}

		return result;
	}

	eae6320::cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight)
	{
		auto result = eae6320::Results::Success;

		ID3D11Texture2D* backBuffer = nullptr;
		ID3D11Texture2D* depthBuffer = nullptr;
		eae6320::cScopeGuard scopeGuard([&backBuffer, &depthBuffer]
			{
				// Regardless of success or failure the two texture resources should be released
				// (if the function is successful the views will hold internal references to the resources)
				if (backBuffer)
				{
					backBuffer->Release();
					backBuffer = nullptr;
				}
				if (depthBuffer)
				{
					depthBuffer->Release();
					depthBuffer = nullptr;
				}
			});

		auto& g_context = eae6320::Graphics::sContext::g_context;
		auto* const direct3dDevice = g_context.direct3dDevice;
		EAE6320_ASSERT(direct3dDevice);
		auto* const direct3dImmediateContext = g_context.direct3dImmediateContext;
		EAE6320_ASSERT(direct3dImmediateContext);

		// Create a "render target view" of the back buffer
		// (the back buffer was already created by the call to D3D11CreateDeviceAndSwapChain(),
		// but a "view" of it is required to use as a "render target",
		// meaning a texture that the GPU can render to)
		{
			// Get the back buffer from the swap chain
			{
				constexpr unsigned int bufferIndex = 0;	// This must be 0 since the swap chain is discarded
				const auto d3dResult = g_context.swapChain->GetBuffer(bufferIndex, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't get the back buffer from the swap chain (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to get the back buffer from the swap chain (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
			// Create the view
			{
				constexpr D3D11_RENDER_TARGET_VIEW_DESC* const accessAllSubResources = nullptr;
				const auto d3dResult = direct3dDevice->CreateRenderTargetView(backBuffer, accessAllSubResources, &eae6320::Graphics::Env::s_renderTargetView);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't create render target view (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create the render target view (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
		}
		// Create a depth/stencil buffer and a view of it
		{
			// Unlike the back buffer no depth/stencil buffer exists until and unless it is explicitly created
			{
				D3D11_TEXTURE2D_DESC textureDescription{};
				{
					textureDescription.Width = i_resolutionWidth;
					textureDescription.Height = i_resolutionHeight;
					textureDescription.MipLevels = 1;	// A depth buffer has no MIP maps
					textureDescription.ArraySize = 1;
					textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24 bits for depth and 8 bits for stencil
					{
						DXGI_SAMPLE_DESC& sampleDescription = textureDescription.SampleDesc;

						sampleDescription.Count = 1;	// No multisampling
						sampleDescription.Quality = 0;	// Doesn't matter when Count is 1
					}
					textureDescription.Usage = D3D11_USAGE_DEFAULT;	// Allows the GPU to write to it
					textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
					textureDescription.CPUAccessFlags = 0;	// CPU doesn't need access
					textureDescription.MiscFlags = 0;
				}
				// The GPU renders to the depth/stencil buffer and so there is no initial data
				// (like there would be with a traditional texture loaded from disk)
				constexpr D3D11_SUBRESOURCE_DATA* const noInitialData = nullptr;
				const auto d3dResult = direct3dDevice->CreateTexture2D(&textureDescription, noInitialData, &depthBuffer);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't create depth buffer (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create the depth buffer resource (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
			// Create the view
			{
				constexpr D3D11_DEPTH_STENCIL_VIEW_DESC* const noSubResources = nullptr;
				const auto d3dResult = direct3dDevice->CreateDepthStencilView(depthBuffer, noSubResources, &eae6320::Graphics::Env::s_depthStencilView);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't create depth stencil view (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create the depth stencil view (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
		}

		// Bind the views
		{
			constexpr unsigned int renderTargetCount = 1;
			direct3dImmediateContext->OMSetRenderTargets(renderTargetCount, &eae6320::Graphics::Env::s_renderTargetView, eae6320::Graphics::Env::s_depthStencilView);
		}
		// Specify that the entire render target should be visible
		{
			D3D11_VIEWPORT viewPort{};
			{
				viewPort.TopLeftX = viewPort.TopLeftY = 0.0f;
				viewPort.Width = static_cast<float>(i_resolutionWidth);
				viewPort.Height = static_cast<float>(i_resolutionHeight);
				viewPort.MinDepth = 0.0f;
				viewPort.MaxDepth = 1.0f;
			}
			constexpr unsigned int viewPortCount = 1;
			direct3dImmediateContext->RSSetViewports(viewPortCount, &viewPort);
		}

		return result;
	}
}

