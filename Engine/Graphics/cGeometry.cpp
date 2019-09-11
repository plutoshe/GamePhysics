#include "cGeometry.h"
#include "PlatformIncludes.h"
#include "GraphicsEnv.h"
#include "sContext.h"
#include <Engine/Asserts/Asserts.h>

namespace eae6320
{
	namespace Graphics
	{
		namespace Geometry
		{
			#if defined( EAE6320_PLATFORM_GL )

				unsigned int* cGeometryIndexFace::GetGeometryIndices()
				{
					return new unsigned int[3]{ m_i0, m_i1, m_i2 };
				}
			#elif defined( EAE6320_PLATFORM_D3D )
				std::vector<unsigned int> cGeometryIndexFace::GetGeometryIndices() const
				{
					return std::vector<unsigned int>{ m_i0, m_i2, m_i1 };
				}
			#endif

			void cGeometryRenderTarget::AddFace(const cGeometryIndexFace &face)
			{
				std::vector<unsigned int> result = face.GetGeometryIndices();
				for (int i = 0; i < 3; i++) 
				{
					m_indices.push_back(result[i]);
				}
			}
			void cGeometryRenderTarget::AddIndices(int FaceNum, const std::vector<unsigned int> &triangleIndices)
			{
				for (int i = 0; i < FaceNum; i++)
				{
					AddFace(cGeometryIndexFace(triangleIndices[i * 3], triangleIndices[i * 3 + 1], triangleIndices[i * 3 + 2]));
				}
			}

			void cGeometryRenderTarget::AddVetices(int vertexNum, const std::vector<cGeometryVertex> &vertices)
			{
				m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.begin() + vertexNum);
			}

			unsigned int cGeometryRenderTarget::VertexBufferSize()
			{
				return static_cast<unsigned int>(m_vertices.size() * sizeof(cGeometryVertex));
			}

			unsigned int cGeometryRenderTarget::IndexBufferSize()
			{
				return static_cast<unsigned int>(m_indices.size() * sizeof(unsigned int));
			}

			cGeometryVertex* cGeometryRenderTarget::GetVertexData()
			{
				return &m_vertices[0];
			}

			unsigned int* cGeometryRenderTarget::GetIndexData()
			{
				return &m_indices[0];
			}

			unsigned int cGeometryRenderTarget::vertexCountToRender()
			{
				return static_cast<unsigned int>(m_vertices.size());
			}

			eae6320::cResult cGeometryRenderTarget::Release()
			{
				auto result = Results::Success;
				if (m_vertexBuffer)
				{
					m_vertexBuffer->Release();
					m_vertexBuffer = nullptr;
				}
				if (m_indexBuffer)
				{
					m_indexBuffer->Release();
					m_indexBuffer = nullptr;

				}
				if (m_vertexFormat)
				{
					const auto result_vertexFormat = cVertexFormat::s_manager.Release(m_vertexFormat);
					if (!result_vertexFormat)
					{
						EAE6320_ASSERT(false);
						if (result)
						{
							result = result_vertexFormat;
						}
					}
				}
				return result;
			}

			void cGeometryRenderTarget::InitData(const std::vector<cGeometryVertex>& vertices, const std::vector<unsigned int>& triangleIndices)
			{
				AddVetices((int)vertices.size(), vertices);
				AddIndices((int)triangleIndices.size() / 3, triangleIndices);
			}

			eae6320::cResult cGeometryRenderTarget::InitDevicePipeline()
			{
#if defined( EAE6320_PLATFORM_D3D )
				eae6320::cResult result = eae6320::Results::Success;

				auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
				EAE6320_ASSERT(direct3dDevice);

				// Vertex Format
				{
					if (!(result = eae6320::Graphics::cVertexFormat::s_manager.Load(eae6320::Graphics::VertexTypes::_3dObject, m_vertexFormat,
						"data/shaders/vertex/vertexinputlayout_3dobject.shader")))
					{
						EAE6320_ASSERTF(false, "Can't initialize geometry without vertex format");
						return result;
					}
				}
				{
					D3D11_BUFFER_DESC bufferDescription{};
					{
						const auto bufferSize = VertexBufferSize();
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
						initialData.pSysMem = GetVertexData();
						// (The other data members are ignored for non-texture buffers)
					}

					auto d3dResult = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &m_vertexBuffer);

					if (FAILED(d3dResult))
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, "3D object vertex buffer creation failed (HRESULT %#010x)", d3dResult);
						eae6320::Logging::OutputError("Direct3D failed to create a 3D object vertex buffer (HRESULT %#010x)", d3dResult);
						return result;
					}

					CD3D11_BUFFER_DESC iDesc(
						(UINT)m_indices.size() * sizeof(unsigned int),
						D3D11_BIND_INDEX_BUFFER
					);

					D3D11_SUBRESOURCE_DATA iData;
					ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
					iData.pSysMem = GetIndexData();
					iData.SysMemPitch = 0;
					iData.SysMemSlicePitch = 0;

					d3dResult = direct3dDevice->CreateBuffer(&iDesc, &iData, &m_indexBuffer);
					if (FAILED(d3dResult))
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, "3D object vertex buffer creation failed (HRESULT %#010x)", d3dResult);
						eae6320::Logging::OutputError("Direct3D failed to create a 3D object vertex buffer (HRESULT %#010x)", d3dResult);
						return result;
					}
				}

				
#elif defined( EAE6320_PLATFORM_GL )
				// Create a vertex array object and make it active
				{
					constexpr GLsizei arrayCount = 1;
					glGenVertexArrays(arrayCount, &vertexArrayId);
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						glBindVertexArray(vertexArrayId);
						const auto errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to bind a new vertex array: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to get an unused vertex array ID: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				// Create a vertex buffer object and make it active
				{
					auto result = eae6320::Results::Success;
					constexpr GLsizei bufferCount = 1;
					glGenBuffers(bufferCount, &eae6320::Graphics::Env::s_vertexBufferId);
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						glBindBuffer(GL_ARRAY_BUFFER, eae6320::Graphics::Env::s_vertexBufferId);
						const auto errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to bind a new vertex buffer: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}

				eae6320::Graphics::Geometry::cGeometryVertex m_vertices[4] =
				{
					eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 0.0f, 0.0f),
					eae6320::Graphics::Geometry::cGeometryVertex(1.0f, 0.0f, 0.0f),
					eae6320::Graphics::Geometry::cGeometryVertex(0.0f, 1.0f, 0.0f),
					eae6320::Graphics::Geometry::cGeometryVertex(1.0f, 1.0f, 0.0f),
				};

				// Assign the data to the vertex position buffer
				{

					const auto bufferSize = 4;
					EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
					glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), reinterpret_cast<GLvoid*>(m_vertices),
						// In our class we won't ever read from the buffer
						GL_STATIC_DRAW);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to allocate the vertex buffer: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}

				// create index buffer and make it active
				{
					auto result = eae6320::Results::Success;
					constexpr GLsizei bufferCount = 1;
					glGenBuffers(bufferCount, &eae6320::Graphics::Env::s_indexBufferId);
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eae6320::Graphics::Env::s_indexBufferId);
						const auto errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to bind a new vertex buffer: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				unsigned int m_indices[] = {  // note that we start from 0!
					0, 1, 2,   // first triangle
					1, 3, 2,   // second triangle
				};

				// Assign the data to the vertex index buffer
				{
					const auto bufferSize = 6;
					EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), reinterpret_cast<GLvoid*>(m_indices),
						// In our class we won't ever read from the buffer
						GL_STATIC_DRAW);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to allocate the vertex buffer: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}



				// Initialize vertex format
				{
					// The "stride" defines how large a single vertex is in the stream of data
					// (or, said another way, how far apart each position element is)
					const auto stride = static_cast<GLsizei>(sizeof(eae6320::Graphics::Geometry::cGeometryVertex));

					// Position (0)
					// 3 floats == 12 bytes
					// Offset = 0
					{
						constexpr GLuint vertexElementLocation = 0;
						constexpr GLint elementCount = 3;
						constexpr GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
						glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride, 0);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							glEnableVertexAttribArray(vertexElementLocation);
							const GLenum errorCode = glGetError();
							if (errorCode != GL_NO_ERROR)
							{
								result = eae6320::Results::Failure;
								EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
								eae6320::Logging::OutputError("OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
									vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
								return result;
							}
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to set the POSITION vertex attribute at location %u: %s",
								vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
				}
				// unbind current vertex array
				{
					glBindVertexArray(0);
				}
#endif
				return result;
			}


			unsigned int cGeometryRenderTarget::GetIndexCount()
			{
				return (unsigned int)m_indices.size();
			}

			void cGeometryRenderTarget::Draw()
			{		
#if defined( EAE6320_PLATFORM_D3D )
				auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
				// Bind a specific vertex buffer to the device as a data source
				{
					EAE6320_ASSERT(m_vertexBuffer);
					EAE6320_ASSERT(m_indexBuffer);
					constexpr unsigned int startingSlot = 0;
					constexpr unsigned int vertexBufferCount = 1;
					// The "stride" defines how large a single vertex is in the stream of data
					constexpr unsigned int bufferStride = sizeof(Graphics::Geometry::cGeometryVertex);
					// It's possible to start streaming data in the middle of a vertex buffer
					constexpr unsigned int bufferOffset = 0;
					direct3dImmediateContext->IASetVertexBuffers(startingSlot, vertexBufferCount, &m_vertexBuffer, &bufferStride, &bufferOffset);
					direct3dImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
				}
				// Specify what kind of data the vertex buffer holds
				{
					// Bind the vertex format (which defines how to interpret a single vertex)
					{
						EAE6320_ASSERT(m_vertexFormat);
						auto* const vertexFormat = cVertexFormat::s_manager.Get(m_vertexFormat);
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
						GetIndexCount(),
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