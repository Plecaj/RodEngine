#pragma once

#include "Rod/Renderer/FrameBuffer.h"

namespace Rod {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		explicit OpenGLFramebuffer(FramebufferSpecification spec);
		~OpenGLFramebuffer() override;
		OpenGLFramebuffer(const OpenGLFramebuffer&) = delete;
		auto operator=(const OpenGLFramebuffer&) -> OpenGLFramebuffer& = delete;
		OpenGLFramebuffer(OpenGLFramebuffer&&) = delete;
		auto operator=(OpenGLFramebuffer&&) -> OpenGLFramebuffer& = delete;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		[[nodiscard]] auto ReadPixel(uint32_t attachmentIndex, int x, int y) -> int override;

		void ClearColorAttachment(uint32_t attachmentIndex, int value) override;

		[[nodiscard]] auto GetColorAttachmentRendererID(uint32_t index = 0) const -> uint32_t override
		{
			RD_CORE_ASSERT(index < m_ColorAttachments.size(), "Index < ColorAttachments.size()");
			return m_ColorAttachments[index];
		}
		[[nodiscard]] auto GetDepthAttachmentRendererID() const noexcept -> uint32_t override
		{
			return m_DepthAttachment;
		}
		void BindDepthAttachment(uint32_t slot = 0) const override;

		[[nodiscard]] auto GetSpecification() const noexcept -> const FramebufferSpecification& override
		{
			return m_Specification;
		};

	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;
	};

}
