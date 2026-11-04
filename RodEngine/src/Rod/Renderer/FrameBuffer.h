#pragma once

#include "Rod/Core/Core.h"

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
		explicit FramebufferTextureSpecification(FramebufferTextureFormat format) : TextureFormat(format) {}

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
		Framebuffer& operator=(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) = delete;
		Framebuffer& operator=(Framebuffer&&) = delete;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearColorAttachment(uint32_t attachmentIndex, int value) = 0;

		[[nodiscard]] virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		[[nodiscard]] virtual uint32_t GetDepthAttachmentRendererID() const noexcept = 0;
		virtual void BindDepthAttachment(uint32_t slot = 0) const = 0;

		[[nodiscard]] virtual const FramebufferSpecification& GetSpecification() const noexcept = 0;

		[[nodiscard]] static Scope<Framebuffer> Create(const FramebufferSpecification& spec);
	};

}
