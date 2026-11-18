#include "rdpch.h"

#include "../OpenGLFixture.h"
#include "Rod/Renderer/Texture.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Textures load RGB and RGBA pixels", "[renderer][opengl]")
{
	std::string filename;
	constexpr uint8_t full = 255;
	constexpr uint8_t half = 128;
	uint8_t whiteAlpha = full;
	SECTION("RGB")
	{
		filename = "rgb.png";
	}
	SECTION("RGBA")
	{
		filename = "rgba.png";
		whiteAlpha = half;
	}
	const auto path = AssetPath(filename).string();
	const auto texture = Rod::Texture2D::Create(path);
	constexpr uint32_t side = 2;
	REQUIRE(texture->GetWidth() == side);
	REQUIRE(texture->GetHeight() == side);

	const auto expected =
	    std::to_array<uint8_t>({0, 0, full, full, full, full, full, whiteAlpha, full, 0, 0, full, 0, full, 0, full});
	std::array<uint8_t, expected.size()> pixels{};
	glGetTextureImage(texture->GetRendererID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, sizeof(pixels), pixels.data());
	REQUIRE(pixels == expected);
	REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Textures upload pixel data", "[renderer][opengl]")
{
	constexpr uint8_t full = 255;
	const auto expected = std::to_array<uint8_t>({full, 0, 0, full, 0, full, 0, full});
	constexpr uint32_t width = 2;
	const auto texture = Rod::Texture2D::Create(width, 1);
	texture->SetData(std::as_bytes(std::span{expected}));

	std::array<uint8_t, expected.size()> pixels{};
	glGetTextureImage(texture->GetRendererID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, sizeof(pixels), pixels.data());
	REQUIRE(pixels == expected);
	REQUIRE(glGetError() == GL_NO_ERROR);
}
