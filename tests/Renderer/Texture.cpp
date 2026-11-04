#include "rdpch.h"

#include "Rod/Platform/OpenGL/OpenGLTexture.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE("Texture loading rejects a missing image", "[renderer]")
{
	REQUIRE_THROWS_AS(Rod::OpenGLTexture2D(std::string{}), std::runtime_error);
}
