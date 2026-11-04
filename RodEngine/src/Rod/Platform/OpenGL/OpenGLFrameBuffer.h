#pragma once

#include "Rod/Renderer/FrameBuffer.h"

namespace Rod {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		explicit OpenGLFramebuffer(FramebufferSpecification spec);
		virtual ~OpenGLFramebuffer() override;
		OpenGLFramebuffer(const OpenGLFramebuffer&) = delete;
		OpenGLFramebuffer& operator=(const OpenGLFramebuffer&) = delete;
		OpenGLFramebuffer(OpenGLFramebuffer&&) = delete;
		OpenGLFramebuffer& operator=(OpenGLFramebuffer&&) = delete;

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearColorAttachment(uint32_t attachmentIndex, int value) override;

		[[nodiscard]] virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override
		{
			RD_CORE_ASSERT(index < m_ColorAttachments.size(), "Index < ColorAttachments.size()");
			return m_ColorAttachments[index];
		}
		[[nodiscard]] virtual uint32_t GetDepthAttachmentRendererID() const noexcept override
		{
			return m_DepthAttachment;
		}
		virtual void BindDepthAttachment(uint32_t slot = 0) const override;

		[[nodiscard]] virtual const FramebufferSpecification& GetSpecification() const noexcept override
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
