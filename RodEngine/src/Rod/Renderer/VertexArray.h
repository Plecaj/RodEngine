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
		auto operator=(const VertexArray&) -> VertexArray& = delete;
		VertexArray(VertexArray&&) = delete;
		auto operator=(VertexArray&&) -> VertexArray& = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(Scope<VertexBuffer> vertexBuffer) = 0;
		virtual void SetIndexBuffer(Scope<IndexBuffer> indexBuffer) = 0;

		[[nodiscard]] virtual auto GetVertexBuffers() const noexcept -> std::span<const Scope<VertexBuffer>> = 0;
		[[nodiscard]] virtual auto GetIndexBuffer() const noexcept -> std::optional<const IndexBuffer&> = 0;

		[[nodiscard]] static auto Create() -> Scope<VertexArray>;
	};

}
