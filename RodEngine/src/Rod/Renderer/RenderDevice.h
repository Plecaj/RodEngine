#pragma once

#include "Rod/Core/Core.h"

#include <glm/vec4.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace Rod {

	class Framebuffer;
	class IndexBuffer;
	class Texture2D;
	class UniformBuffer;
	class VertexArray;
	class VertexBuffer;
	struct FramebufferSpecification;

	class RenderDevice
	{
	public:
		void ConfigureDefaults() const;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
		void Clear(const glm::vec4& color) const;
		void ClearDepth() const;
		void DrawIndexed(const VertexArray& vertexArray) const;
		void DrawIndexed(const VertexArray& vertexArray, uint32_t indexCount) const;

		struct RenderTarget
		{
			uint32_t DrawFramebuffer = 0;
			uint32_t ReadFramebuffer = 0;
			int32_t X = 0;
			int32_t Y = 0;
			uint32_t Width = 0;
			uint32_t Height = 0;
		};

		[[nodiscard]] auto GetRenderTarget() const -> RenderTarget;
		void SetRenderTarget(const RenderTarget& target) const;

		[[nodiscard]] auto CreateFramebuffer(const FramebufferSpecification& spec) const -> Scope<Framebuffer>;
		[[nodiscard]] auto CreateVertexArray() const -> Scope<VertexArray>;
		[[nodiscard]] auto CreateVertexBuffer(uint32_t size) const -> Scope<VertexBuffer>;
		[[nodiscard]] auto CreateVertexBuffer(std::span<const std::byte> vertices) const -> Scope<VertexBuffer>;
		[[nodiscard]] auto CreateIndexBuffer(std::span<const uint32_t> indices) const -> Scope<IndexBuffer>;
		[[nodiscard]] auto CreateUniformBuffer(uint32_t size) const -> Scope<UniformBuffer>;
		[[nodiscard]] auto CreateTexture2D(uint32_t width, uint32_t height) const -> Scope<Texture2D>;
		[[nodiscard]] auto LoadTexture2D(const std::string& path) const -> Scope<Texture2D>;
	};

}
