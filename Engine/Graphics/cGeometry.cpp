#include "cGeometry.h"
#include "PlatformIncludes.h"
#include "GraphicsEnv.h"
#include <Engine/Asserts/Asserts.h>

namespace eae6320
{
	namespace Graphics
	{
		namespace Geometry
		{
			#if defined( EAE6320_PLATFORM_GL )

				cGeometryVertex* cGeometryFace::GetGeometryVertices()
				{
					return new cGeometryVertex[3]{ m_v0, m_v1, m_v2 };
				}
			#elif defined( EAE6320_PLATFORM_D3D )
				cGeometryVertex* cGeometryFace::GetGeometryVertices()
				{
					return new cGeometryVertex[3]{ m_v0, m_v2, m_v1 };
				}
			#endif

			void cGeometryRenderTarget::AddFace(cGeometryFace face)
			{
				cGeometryVertex* result = face.GetGeometryVertices();
				for (int i = 0; i < 3; i++) 
				{
					vertices.push_back(result[i]);
				}
			}

			unsigned int cGeometryRenderTarget::BufferSize()
			{
				return static_cast<unsigned int>(vertices.size() * sizeof(cGeometryVertex));
			}

			cGeometryVertex* cGeometryRenderTarget::GetVertexData()
			{
				return &vertices[0];
			}

			unsigned int cGeometryRenderTarget::vertexCountToRender()
			{
				return static_cast<unsigned int>(vertices.size());
			}

			void cGeometryRenderTarget::Draw()
			{		
#if defined( EAE6320_PLATFORM_D3D )
				auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
				// Bind a specific vertex buffer to the device as a data source
				{
					EAE6320_ASSERT(eae6320::Graphics::Env::s_vertexBuffer);
					EAE6320_ASSERT(eae6320::Graphics::Env::s_indexBuffer);
					constexpr unsigned int startingSlot = 0;
					constexpr unsigned int vertexBufferCount = 1;
					// The "stride" defines how large a single vertex is in the stream of data
					constexpr unsigned int bufferStride = sizeof(Graphics::Geometry::cGeometryVertex);
					// It's possible to start streaming data in the middle of a vertex buffer
					constexpr unsigned int bufferOffset = 0;
					direct3dImmediateContext->IASetVertexBuffers(startingSlot, vertexBufferCount, &eae6320::Graphics::Env::s_vertexBuffer, &bufferStride, &bufferOffset);

					direct3dImmediateContext->IASetIndexBuffer(eae6320::Graphics::Env::s_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
				}
				// Specify what kind of data the vertex buffer holds
				{
					// Bind the vertex format (which defines how to interpret a single vertex)
					{
						EAE6320_ASSERT(eae6320::Graphics::Env::s_vertexFormat);
						auto* const vertexFormat = cVertexFormat::s_manager.Get(eae6320::Graphics::Env::s_vertexFormat);
						EAE6320_ASSERT(vertexFormat);
						vertexFormat->Bind();
					}
					// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
					// the vertex buffer was defined as a triangle list
					// (meaning that every primitive is a triangle and will be defined by three vertices)
					direct3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}
				// Render triangles from the currently-bound vertex buffer
				{
					// It's possible to start rendering primitives in the middle of the stream
					constexpr unsigned int indexOfFirstVertexToRender = 0;
					//direct3dImmediateContext->Draw(vertexCountToRender(), indexOfFirstVertexToRender);
					direct3dImmediateContext->DrawIndexed(
						6,
						0,
						0
					);
				}
#elif defined( EAE6320_PLATFORM_GL )
				// Bind a specific vertex buffer to the device as a data source
				{
					EAE6320_ASSERT(eae6320::Graphics::Env::s_vertexArrayId != 0);
					glBindVertexArray(eae6320::Graphics::Env::s_vertexArrayId);
					EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
				}
				// Render triangles from the currently-bound vertex buffer
				{
					// The mode defines how to interpret multiple vertices as a single "primitive";
					// a triangle list is defined
					// (meaning that every primitive is a triangle and will be defined by three vertices)
					constexpr GLenum mode = GL_TRIANGLES;
					// As of this comment only a single triangle is drawn
					// (you will have to update this code in future assignments!)

					// It's possible to start rendering primitives in the middle of the stream
					constexpr unsigned int indexOfFirstVertexToRender = 0;
					glDrawElements(mode, vertexCountToRender(), GL_UNSIGNED_INT, indexOfFirstVertexToRender);
					EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
				}

				// unbind vertex array
				{
					glBindVertexArray(0);
				}
#endif
			}
		}
	}
}