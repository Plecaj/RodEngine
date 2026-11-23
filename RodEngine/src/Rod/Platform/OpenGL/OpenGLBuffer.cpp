#include "rdpch.h"

#include "OpenGLBuffer.h"

#include <glad/glad.h>

#include <limits>
#include <utility>

namespace Rod {

	/// =========================================
	/// Vertex buffer
	/// =========================================

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		RD_CORE_ASSERT(std::in_range<GLsizeiptr>(size), "Buffer size exceeds OpenGL size limits");
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(std::span<const std::byte> vertices)
	{
		RD_CORE_ASSERT(std::in_range<GLsizeiptr>(vertices.size_bytes()), "Buffer size exceeds OpenGL size limits");
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(std::span<const std::byte> data)
	{
		RD_CORE_ASSERT(std::in_range<GLsizeiptr>(data.size_bytes()), "Buffer size exceeds OpenGL size limits");
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(data.size_bytes()), data.data());
	}

	/// ========================================
	/// Index buffer
	/// ========================================

	OpenGLIndexBuffer::OpenGLIndexBuffer(std::span<const uint32_t> indices)
	{
		RD_CORE_ASSERT(std::in_range<uint32_t>(indices.size()), "Index count exceeds buffer count limits");
		RD_CORE_ASSERT(
		    indices.size() <= static_cast<std::size_t>((std::numeric_limits<GLsizeiptr>::max)()) / sizeof(uint32_t),
		    "Index buffer size exceeds OpenGL size limits");
		m_Count = static_cast<uint32_t>(indices.size());
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(
		    GL_ELEMENT_ARRAY_BUFFER,
		    static_cast<GLsizeiptr>(indices.size_bytes()),
		    indices.data(),
		    GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	/// ========================================
	/// Uniform buffer
	/// ========================================

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size)
	{
		RD_CORE_ASSERT(std::in_range<GLsizeiptr>(size), "Buffer size exceeds OpenGL size limits");
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::Bind() const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
	}

	void OpenGLUniformBuffer::Unbind() const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLUniformBuffer::BindBase(uint32_t bindingIndex) const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(std::span<const std::byte> data)
	{
		RD_CORE_ASSERT(std::in_range<GLsizeiptr>(data.size_bytes()), "Buffer size exceeds OpenGL size limits");
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(data.size_bytes()), data.data());
	}

}
