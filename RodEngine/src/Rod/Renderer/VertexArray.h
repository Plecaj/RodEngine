#pragma once

#include "Buffer.h"

#include <optional>
#include <span>

namespace Rod {

	class VertexArray
	{
	public:
		VertexArray() = default;
		virtual ~VertexArray() = default;
		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;
		VertexArray(VertexArray&&) = delete;
		VertexArray& operator=(VertexArray&&) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(Scope<VertexBuffer> vertexBuffer) = 0;
		virtual void SetIndexBuffer(Scope<IndexBuffer> indexBuffer) = 0;

		[[nodiscard]] virtual std::span<const Scope<VertexBuffer>> GetVertexBuffers() const noexcept = 0;
		[[nodiscard]] virtual std::optional<const IndexBuffer&> GetIndexBuffer() const noexcept = 0;

		[[nodiscard]] static Scope<VertexArray> Create();
	};

}
