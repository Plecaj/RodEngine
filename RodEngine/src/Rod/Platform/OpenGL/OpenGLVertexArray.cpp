#include "rdpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Rod {

	static uint32_t ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Rod::ShaderDataType::Float:  return GL_FLOAT;
			case Rod::ShaderDataType::Float2: return GL_FLOAT;
			case Rod::ShaderDataType::Float3: return GL_FLOAT;
			case Rod::ShaderDataType::Float4: return GL_FLOAT;
			case Rod::ShaderDataType::Mat3:   return GL_FLOAT;
			case Rod::ShaderDataType::Mat4:   return GL_FLOAT;
			case Rod::ShaderDataType::Int:    return GL_INT;
			case Rod::ShaderDataType::Int2:   return GL_INT;
			case Rod::ShaderDataType::Int3:   return GL_INT;
			case Rod::ShaderDataType::Int4:   return GL_INT;
			case Rod::ShaderDataType::Bool:   return GL_BOOL;
		}

		RD_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		RD_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		RD_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		RD_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		RD_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		RD_PROFILE_FUNCTION();

		RD_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!")

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGLBaseType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)element.Offset);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		RD_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}