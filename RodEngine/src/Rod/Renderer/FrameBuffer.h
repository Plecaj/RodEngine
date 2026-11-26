#pragma once

#include "Rod/Core/Core.h"

#include <glm/vec4.hpp>

#include <cstdint>
#include <initializer_list>
#include <vector>

namespace Rod {

	enum class FramebufferTextureFormat : uint8_t
	{
		None = 0,

		// Color
		RGBA8,
		RedInteger,

		// Depth
		DEPTH24,

		// Depth/Stencil
		DEPTH24STENCIL8,
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		constexpr explicit FramebufferTextureSpecification(FramebufferTextureFormat format) noexcept
		    : TextureFormat(format)
		{}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		// TODO: filtering/wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
		    : Attachments(attachments)
		{}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		Framebuffer() = default;
		virtual ~Framebuffer() = default;
		Framebuffer(const Framebuffer&) = delete;
		auto operator=(const Framebuffer&) -> Framebuffer& = delete;
		Framebuffer(Framebuffer&&) = delete;
		auto operator=(Framebuffer&&) -> Framebuffer& = delete;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		[[nodiscard]] virtual auto ReadPixel(uint32_t attachmentIndex, int x, int y) -> int = 0;

		virtual void ClearColorAttachment(uint32_t attachmentIndex, const glm::vec4& color) = 0;
		virtual void ClearColorAttachment(uint32_t attachmentIndex, int value) = 0;

		[[nodiscard]] virtual auto GetColorAttachmentRendererID(uint32_t index = 0) const -> uint32_t = 0;
		[[nodiscard]] virtual auto GetDepthAttachmentRendererID() const noexcept -> uint32_t = 0;
		virtual void BindDepthAttachment(uint32_t slot = 0) const = 0;

		[[nodiscard]] virtual auto GetSpecification() const noexcept -> const FramebufferSpecification& = 0;
	};

}
