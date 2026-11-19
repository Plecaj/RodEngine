#pragma once

#include "Rod/Renderer/VertexArray.h"

#include <optional>
#include <span>
#include <vector>

namespace Rod {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray() override;
		OpenGLVertexArray(const OpenGLVertexArray&) = delete;
		OpenGLVertexArray& operator=(const OpenGLVertexArray&) = delete;
		OpenGLVertexArray(OpenGLVertexArray&&) = delete;
		OpenGLVertexArray& operator=(OpenGLVertexArray&&) = delete;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(Scope<VertexBuffer> vertexBuffer) override;
		virtual void SetIndexBuffer(Scope<IndexBuffer> indexBuffer) override;

		[[nodiscard]] virtual std::span<const Scope<VertexBuffer>> GetVertexBuffers() const noexcept override
		{
			return m_VertexBuffers;
		}
		[[nodiscard]] std::optional<const IndexBuffer&> GetIndexBuffer() const noexcept override
		{
			if (m_IndexBuffer)
				return *m_IndexBuffer;
			return std::nullopt;
		}

	private:
		uint32_t m_RendererID = 0;
		std::vector<Scope<VertexBuffer>> m_VertexBuffers;
		Scope<IndexBuffer> m_IndexBuffer;

		int m_VertexBufferIndex = 0;
	};

}
