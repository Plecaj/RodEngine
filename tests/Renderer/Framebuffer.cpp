#include "rdpch.h"

#include "Rod/Renderer/FrameBuffer.h"

#include "../OpenGLFixture.h"
#include "Rod/Renderer/RenderDevice.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Framebuffers retain attachments after resize", "[renderer][opengl]")
{
	Rod::FramebufferSpecification spec;
	spec.Width = 32;
	spec.Height = 16;
	spec.Attachments = {
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RGBA8},
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RedInteger},
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::DEPTH24STENCIL8},
	};
	const auto framebuffer = Rod::RenderDevice{}.CreateFramebuffer(spec);

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
	for (const GLuint attachment : {
	         framebuffer->GetColorAttachmentRendererID(),
	         framebuffer->GetColorAttachmentRendererID(1),
	         framebuffer->GetDepthAttachmentRendererID(),
	     })
	{
		glGetTextureLevelParameteriv(attachment, 0, GL_TEXTURE_WIDTH, &attachmentWidth);
		glGetTextureLevelParameteriv(attachment, 0, GL_TEXTURE_HEIGHT, &attachmentHeight);
		REQUIRE(std::cmp_equal(attachmentWidth, spec.Width));
		REQUIRE(std::cmp_equal(attachmentHeight, spec.Height));
	}
	framebuffer->Unbind();
	framebuffer->ClearColorAttachment(1, -1);
	framebuffer->ClearColorAttachment(0, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	framebuffer->Bind();
	REQUIRE(framebuffer->ReadPixel(1, 0, 0) == -1);
	framebuffer->ClearColorAttachment(1, 42);
	REQUIRE(framebuffer->ReadPixel(1, static_cast<int>(spec.Width - 1), static_cast<int>(spec.Height - 1)) == 42);
	std::array<uint8_t, 4> pixel{};
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
	REQUIRE(pixel == std::array<uint8_t, 4>{255, 0, 0, 255});
	framebuffer->Unbind();
	REQUIRE(glGetError() == GL_NO_ERROR);
}
