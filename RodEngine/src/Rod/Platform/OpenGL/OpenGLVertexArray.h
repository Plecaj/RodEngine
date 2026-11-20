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
		~OpenGLVertexArray() override;
		OpenGLVertexArray(const OpenGLVertexArray&) = delete;
		auto operator=(const OpenGLVertexArray&) -> OpenGLVertexArray& = delete;
		OpenGLVertexArray(OpenGLVertexArray&&) = delete;
		auto operator=(OpenGLVertexArray&&) -> OpenGLVertexArray& = delete;

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(Scope<VertexBuffer> vertexBuffer) override;
		void SetIndexBuffer(Scope<IndexBuffer> indexBuffer) override;

		[[nodiscard]] auto GetVertexBuffers() const noexcept -> std::span<const Scope<VertexBuffer>> override
		{
			return m_VertexBuffers;
		}
		[[nodiscard]] auto GetIndexBuffer() const noexcept -> std::optional<const IndexBuffer&> override
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
