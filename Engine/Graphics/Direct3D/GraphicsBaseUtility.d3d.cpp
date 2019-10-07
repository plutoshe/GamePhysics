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
#include <vector>

// Helper Function Declarations
//=============================

namespace
{
	eae6320::cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);
}

void eae6320::Graphics::ClearBackgroundColor()
{
	if (eae6320::Graphics::Env::s_BackgroundColor.size() >= 4)
	{
		auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
		float clearColor[4] = { eae6320::Graphics::Env::s_BackgroundColor[0],
								eae6320::Graphics::Env::s_BackgroundColor[1],
								eae6320::Graphics::Env::s_BackgroundColor[2],
								eae6320::Graphics::Env::s_BackgroundColor[3] };

		direct3dImmediateContext->ClearRenderTargetView(eae6320::Graphics::Env::s_renderTargetView, clearColor);
	}
}

void eae6320::Graphics::PrepocessBeforeRender()
{
	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		EAE6320_ASSERT(eae6320::Graphics::Env::s_renderTargetView);

		eae6320::Graphics::ClearBackgroundColor();
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
}

void eae6320::Graphics::PostpocessAfterRender()
{
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

eae6320::cResult eae6320::Graphics::PlatformSpecificInitialization(const sInitializationParameters& i_initializationParameters)
{
	auto result = Results::Success;

	if (!(result = cVertexFormat::s_manager.Initialize()))
	{
		EAE6320_ASSERTF(false, "Can't initialize Graphics without the vertex format manager");
		return result;
	}
	
	// Initialize the views
	{
		if (!(result = InitializeViews(i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the views");
			return result;
		}
	}
	

	return result;
}
namespace eae6320
{
	namespace Graphics
	{
		eae6320::cResult PlatformCleanUp()
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

			return result;
		}
	}
}


// Helper Function Definitions
//============================

eae6320::cResult eae6320::Graphics::InitializeShadingData()
{
	auto result = eae6320::Results::Success;
	// Vertex Format
	{
		if (!(result = eae6320::Graphics::cVertexFormat::s_manager.Load(eae6320::Graphics::VertexTypes::_3dObject, eae6320::Graphics::Env::s_vertexFormat,
			"data/shaders/vertex/vertexinputlayout_3dobject.shader")))
		{
			EAE6320_ASSERTF(false, "Can't initialize geometry without vertex format");
			return result;
		}
	}
	return result;
}


namespace
{


	eae6320::cResult LoadShaderData(
		std::string path,
		std::map<std::string, eae6320::Graphics::cShader::Handle>& shaderMap,
		eae6320::Graphics::ShaderTypes::eType shaderType)
	{
		auto result = eae6320::Results::Success;
		if (path == "")
		{
			EAE6320_ASSERTF(false, "Load Shader failed");
			return  eae6320::Results::Failure;
		}
		if (shaderMap.find(path) == shaderMap.end())
		{
			shaderMap.insert(std::pair<std::string, eae6320::Graphics::cShader::Handle>(path, eae6320::Graphics::cShader::Handle()));
			if (!(result = eae6320::Graphics::cShader::s_manager.Load(
				path,
				shaderMap[path],
				shaderType)))
			{
				EAE6320_ASSERTF(false, "Load Shader failed");
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

