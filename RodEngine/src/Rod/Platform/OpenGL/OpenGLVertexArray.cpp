#include "rdpch.h"

#include "OpenGLVertexArray.h"

#include "Rod/Renderer/Buffer.h"

#include <glad/glad.h>

namespace Rod {

	namespace {
		auto ShaderDataTypeToOpenGLBaseType(ShaderDataType type) -> uint32_t
		{
			switch (type)
			{
				case Rod::ShaderDataType::Float:
				case Rod::ShaderDataType::Float2:
				case Rod::ShaderDataType::Float3:
				case Rod::ShaderDataType::Float4:
				case Rod::ShaderDataType::Mat3:
				case Rod::ShaderDataType::Mat4:
					return GL_FLOAT;
				case Rod::ShaderDataType::Int:
				case Rod::ShaderDataType::Int2:
				case Rod::ShaderDataType::Int3:
				case Rod::ShaderDataType::Int4:
					return GL_INT;
				case Rod::ShaderDataType::Bool:
					return GL_BOOL;
				case Rod::ShaderDataType::None:
					break;
			}

			RD_CORE_ASSERT(false, "Unknown ShaderDataType");
			return 0;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(Scope<VertexBuffer> vertexBuffer)
	{
		const auto& layout = vertexBuffer->GetLayout();
		RD_CORE_ASSERT(!layout.GetElements().empty(), "Vertex Buffer has no layout!")

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4: {
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(
					    m_VertexBufferIndex,
					    static_cast<GLint>(element.GetComponentCount()),
					    ShaderDataTypeToOpenGLBaseType(element.Type),
					    element.Normalized ? GL_TRUE : GL_FALSE,
					    static_cast<GLsizei>(layout.GetStride()),
					    reinterpret_cast<const void*>(static_cast<uintptr_t>(element.Offset)));
					m_VertexBufferIndex++;
					break;
				}
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool: {
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribIPointer(
					    m_VertexBufferIndex,
					    static_cast<GLint>(element.GetComponentCount()),
					    ShaderDataTypeToOpenGLBaseType(element.Type),
					    static_cast<GLsizei>(layout.GetStride()),
					    reinterpret_cast<const void*>(static_cast<uintptr_t>(element.Offset)));
					m_VertexBufferIndex++;
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				case ShaderDataType::None:
					RD_CORE_ASSERT(false, "Unsupported vertex attribute type");
					break;
			}
		}
		// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
		m_VertexBuffers.push_back(std::move(vertexBuffer));
	}

	void OpenGLVertexArray::SetIndexBuffer(Scope<IndexBuffer> indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = std::move(indexBuffer);
	}

}
