#include "rdpch.h"

#include "Rod/Renderer/FrameBuffer.h"

#include "../OpenGLFixture.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Framebuffers retain attachments after resize", "[renderer][opengl]")
{
	constexpr uint32_t width = 32;
	constexpr uint32_t height = 16;
	Rod::FramebufferSpecification spec;
	spec.Width = width;
	spec.Height = height;
	spec.Attachments = {
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RGBA8},
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RedInteger},
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::DEPTH24STENCIL8}};
	const auto framebuffer = Rod::Framebuffer::Create(spec);

	SECTION("initial size")
	{}
	SECTION("resized")
	{
		std::swap(spec.Width, spec.Height);
		framebuffer->Resize(spec.Width, spec.Height);
	}

	framebuffer->Bind();
	REQUIRE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	REQUIRE(framebuffer->GetSpecification().Width == spec.Width);
	REQUIRE(framebuffer->GetSpecification().Height == spec.Height);
	GLint attachmentWidth = 0;
	GLint attachmentHeight = 0;
	for (const GLuint attachment :
	     {framebuffer->GetColorAttachmentRendererID(),
	      framebuffer->GetColorAttachmentRendererID(1),
	      framebuffer->GetDepthAttachmentRendererID()})
	{
		glGetTextureLevelParameteriv(attachment, 0, GL_TEXTURE_WIDTH, &attachmentWidth);
		glGetTextureLevelParameteriv(attachment, 0, GL_TEXTURE_HEIGHT, &attachmentHeight);
		REQUIRE(std::cmp_equal(attachmentWidth, spec.Width));
		REQUIRE(std::cmp_equal(attachmentHeight, spec.Height));
	}
	constexpr uint32_t entityAttachment = 1;
	constexpr int entityID = 42;
	framebuffer->ClearColorAttachment(entityAttachment, -1);
	REQUIRE(framebuffer->ReadPixel(entityAttachment, 0, 0) == -1);
	framebuffer->ClearColorAttachment(entityAttachment, entityID);
	REQUIRE(
	    framebuffer->ReadPixel(entityAttachment, static_cast<int>(spec.Width - 1), static_cast<int>(spec.Height - 1)) ==
	    entityID);
	framebuffer->Unbind();
	REQUIRE(glGetError() == GL_NO_ERROR);
}
