#include "rdpch.h"

#include "../OpenGLFixture.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Renderer/Texture.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Textures load RGB and RGBA pixels", "[renderer][opengl]")
{
	std::string filename;
	uint8_t whiteAlpha = 255;
	SECTION("RGB")
	{
		filename = "rgb.png";
	}
	SECTION("RGBA")
	{
		filename = "rgba.png";
		whiteAlpha = 128;
	}
	const auto path = AssetPath(filename).string();
	const auto texture = Rod::RenderDevice{}.LoadTexture2D(path);
	REQUIRE(texture->GetWidth() == 2);
	REQUIRE(texture->GetHeight() == 2);

	const auto expected =
	    std::to_array<uint8_t>({0, 0, 255, 255, 255, 255, 255, whiteAlpha, 255, 0, 0, 255, 0, 255, 0, 255});
	std::array<uint8_t, expected.size()> pixels{};
	glGetTextureImage(texture->GetRendererID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, sizeof(pixels), pixels.data());
	REQUIRE(pixels == expected);
	REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Textures upload pixel data", "[renderer][opengl]")
{
	const auto expected = std::to_array<uint8_t>({255, 0, 0, 255, 0, 255, 0, 255});
	const auto texture = Rod::RenderDevice{}.CreateTexture2D(2, 1);
	texture->SetData(std::as_bytes(std::span{expected}));

	std::array<uint8_t, expected.size()> pixels{};
	glGetTextureImage(texture->GetRendererID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, sizeof(pixels), pixels.data());
	REQUIRE(pixels == expected);
	REQUIRE(glGetError() == GL_NO_ERROR);
}
