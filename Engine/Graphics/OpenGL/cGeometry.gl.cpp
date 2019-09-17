#include "../cGeometry.h"
#include "../GraphicsEnv.h"
// init data pipeline
namespace eae6320
{
	namespace Graphics
	{
		namespace Geometry
		{
			eae6320::cResult cGeometryRenderTarget::InitDevicePipeline()
			{
				auto result = eae6320::Results::Success;
				if (!m_isInitialized)
				{
					m_isInitialized = true;

					// Create a vertex array object and make it active
					{
						constexpr GLsizei arrayCount = 1;
						glGenVertexArrays(arrayCount, &m_vertexArrayId);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							glBindVertexArray(m_vertexArrayId);
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
						glGenBuffers(bufferCount, &m_vertexBufferId);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
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

					// Assign the data to the vertex position buffer
					{

						const auto bufferSize = m_vertices.size();
						EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
						glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(eae6320::Graphics::Geometry::cGeometryVertex), reinterpret_cast<GLvoid*>(GetVertexData()),
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
						glGenBuffers(bufferCount, &m_indexBufferId);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
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

					// Assign the data to the vertex index buffer
					{
						const auto bufferSize = GetIndexCount();
						EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetIndexCount() * sizeof(unsigned int), reinterpret_cast<GLvoid*>(GetIndexData()),
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
				}
				return result;
			}

			eae6320::cResult cGeometryRenderTarget::Release()
			{
				auto result = eae6320::Results::Success;
				if (m_vertexArrayId != 0)
				{
					// Make sure that the vertex array isn't bound
					{
						// Unbind the vertex array
						glBindVertexArray(0);
						const auto errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							if (result)
							{
								result = Results::Failure;
							}
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							Logging::OutputError("OpenGL failed to unbind all vertex arrays before cleaning up geometry: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
						}
					}
					constexpr GLsizei arrayCount = 1;
					glDeleteVertexArrays(arrayCount, &m_vertexArrayId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						if (result)
						{
							result = Results::Failure;
						}
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						Logging::OutputError("OpenGL failed to delete the vertex array: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
					}
					m_vertexArrayId = 0;
				}
				if (m_vertexBufferId != 0)
				{
					constexpr GLsizei bufferCount = 1;
					glDeleteBuffers(bufferCount, &m_vertexBufferId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						if (result)
						{
							result = Results::Failure;
						}
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						Logging::OutputError("OpenGL failed to delete the vertex buffer: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
					}
					m_vertexBufferId = 0;
				}
				return result;
			}

			void cGeometryRenderTarget::Draw()
			{

				// Bind a specific vertex buffer to the device as a data source
				{
					
					EAE6320_ASSERT(m_vertexArrayId != 0);
					glBindVertexArray(m_vertexArrayId);
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
					glDrawElements(mode, 6, GL_UNSIGNED_INT, indexOfFirstVertexToRender);
					EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
				}

				// unbind vertex array
				{
					glBindVertexArray(0);
				}

			}
		}
	}
}