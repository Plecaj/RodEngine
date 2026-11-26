#include "rdpch.h"

#include "Rod/Renderer/Renderer2D.h"

#include "../OpenGLFixture.h"
#include "../TemporaryDirectory.h"
#include "Rod/Renderer/FrameBuffer.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Renderer/ShaderLoader.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

#include <array>
#include <span>

namespace {
	auto CreateSpriteTarget(const Rod::RenderDevice& device) -> Rod::Scope<Rod::Framebuffer>
	{
		Rod::FramebufferSpecification spec;
		spec.Width = 8;
		spec.Height = 8;
		spec.Attachments = {
		    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RGBA8},
		    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RedInteger},
		};
		return device.CreateFramebuffer(spec);
	}

	void ClearSpriteTarget(Rod::Framebuffer& target)
	{
		target.ClearColorAttachment(0, glm::vec4(0.0f));
		target.ClearColorAttachment(1, -1);
	}

	auto ReadColor(int x, int y) -> std::array<uint8_t, 4>
	{
		std::array<uint8_t, 4> pixel{};
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
		return pixel;
	}
}

TEST_CASE_METHOD(
    Rod::Tests::OpenGLFixture,
    "Sprite batch preserves the texture and picking ID across a flush",
    "[renderer][opengl]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::RenderDevice device;
	const Rod::ShaderLoader loader(ROD_TEST_SHADER_DIRECTORY, directory.GetPath());
	Rod::Renderer2D sprites(device, loader);

	const auto target = CreateSpriteTarget(device);
	target->Bind();
	ClearSpriteTarget(*target);

	constexpr glm::vec4 white(1.0f);
	glm::mat4 outside(1.0f);
	outside[3].x = 10.0f;
	sprites.Begin(glm::mat4(1.0f));
	for (int index = 0; index < 31; ++index)
	{
		const Rod::Ref<Rod::Texture2D> texture = device.CreateTexture2D(1, 1);
		sprites.SubmitQuad(outside, texture, 1.0f, white);
	}

	const Rod::Ref<Rod::Texture2D> blue = device.CreateTexture2D(1, 1);
	constexpr std::array<uint8_t, 4> bluePixel{0, 0, 255, 255};
	blue->SetData(std::as_bytes(std::span{bluePixel}));
	sprites.SubmitQuad(glm::mat4(1.0f), blue, 1.0f, white, 42);
	sprites.End();

	REQUIRE(sprites.GetStats().DrawCalls == 2);
	REQUIRE(sprites.GetStats().QuadCount == 32);
	REQUIRE(target->ReadPixel(1, 4, 4) == 42);
	REQUIRE(ReadColor(4, 4) == bluePixel);
	REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Mixed sprite batches reset between frames", "[renderer][opengl]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::RenderDevice device;
	const Rod::ShaderLoader loader(ROD_TEST_SHADER_DIRECTORY, directory.GetPath());
	Rod::Renderer2D sprites(device, loader);
	const auto target = CreateSpriteTarget(device);
	target->Bind();
	ClearSpriteTarget(*target);

	constexpr glm::vec4 white(1.0f);
	constexpr glm::vec4 green(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr glm::vec4 red(1.0f, 0.0f, 0.0f, 1.0f);
	constexpr std::array<uint8_t, 4> yellowPixel{255, 255, 0, 255};
	constexpr std::array<uint8_t, 4> greenPixel{0, 255, 0, 255};
	constexpr std::array<uint8_t, 4> redPixel{255, 0, 0, 255};
	const Rod::Ref<Rod::Texture2D> texture = device.CreateTexture2D(1, 1);
	texture->SetData(std::as_bytes(std::span{yellowPixel}));

	glm::mat4 center(1.0f);
	center[0].x = 0.5f;
	auto left = center;
	left[3].x = -0.5f;
	auto right = center;
	right[3].x = 0.5f;

	sprites.Begin(glm::mat4(1.0f));
	sprites.SubmitQuad(left, texture, 1.0f, green, 1);
	sprites.SubmitQuad(center, texture, 1.0f, white, 2);
	sprites.SubmitQuad(right, {}, 1.0f, red, 3);
	sprites.End();

	REQUIRE(sprites.GetStats().DrawCalls == 1);
	REQUIRE(sprites.GetStats().QuadCount == 3);
	REQUIRE(target->ReadPixel(1, 2, 4) == 1);
	REQUIRE(target->ReadPixel(1, 4, 4) == 2);
	REQUIRE(target->ReadPixel(1, 6, 4) == 3);
	REQUIRE(ReadColor(2, 4) == greenPixel);
	REQUIRE(ReadColor(4, 4) == yellowPixel);
	REQUIRE(ReadColor(6, 4) == redPixel);

	ClearSpriteTarget(*target);
	sprites.Begin(glm::mat4(1.0f));
	sprites.SubmitQuad(center, texture, 1.0f, white, 2);
	sprites.End();

	REQUIRE(sprites.GetStats().DrawCalls == 1);
	REQUIRE(sprites.GetStats().QuadCount == 1);
	REQUIRE(target->ReadPixel(1, 2, 4) == -1);
	REQUIRE(target->ReadPixel(1, 4, 4) == 2);
	REQUIRE(target->ReadPixel(1, 6, 4) == -1);
	REQUIRE(glGetError() == GL_NO_ERROR);
}
