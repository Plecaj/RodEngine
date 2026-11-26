#include "rdpch.h"

#include "Rod/Renderer/Shader.h"

#include "../OpenGLFixture.h"
#include "../TemporaryDirectory.h"
#include "Rod/Renderer/ShaderLoader.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

#include <chrono>

namespace {
	void RequireLinkedShaders(const Rod::ShaderLoader& loader)
	{
		for (const auto* filename : {"Texture.glsl", "3D.glsl", "Shadow.glsl"})
		{
			CAPTURE(filename);
			const auto shader = loader.Load(filename);
			shader->Bind();
			GLint program = 0;
			glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			REQUIRE(program > 0);
			GLint linked = GL_FALSE;
			glGetProgramiv(static_cast<GLuint>(program), GL_LINK_STATUS, &linked);
			REQUIRE(linked == GL_TRUE);
			shader->Unbind();
			REQUIRE(glGetError() == GL_NO_ERROR);
		}
	}
}

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Engine shaders compile and reload from cache", "[renderer][opengl]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const auto cacheDirectory = directory.GetPath() / "cache";
	RequireLinkedShaders(Rod::ShaderLoader(ROD_TEST_SHADER_DIRECTORY, cacheDirectory));

	std::vector<std::pair<std::filesystem::path, std::filesystem::file_time_type>> cachedFiles;
	const auto timestamp = std::filesystem::file_time_type::clock::now() - std::chrono::hours{24};
	for (const auto& entry : std::filesystem::directory_iterator(cacheDirectory))
	{
		std::filesystem::last_write_time(entry.path(), timestamp);
		cachedFiles.emplace_back(entry.path(), std::filesystem::last_write_time(entry.path()));
	}
	REQUIRE(cachedFiles.size() == 6);

	RequireLinkedShaders(Rod::ShaderLoader(ROD_TEST_SHADER_DIRECTORY, cacheDirectory));
	for (const auto& [path, modified] : cachedFiles)
	{
		CAPTURE(path);
		// libc++ can store filesystem timestamps with a 128-bit tick count.
		REQUIRE((std::filesystem::last_write_time(path) == modified));
	}
}

TEST_CASE_METHOD(
    Rod::Tests::OpenGLFixture,
    "Shader compile options select separate cached artifacts",
    "[renderer][shader-cache][opengl]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::ShaderLoader loader(ROD_TEST_SHADER_DIRECTORY, directory.GetPath());
	const auto regular = loader.Load("3D.glsl");
	Rod::ShaderCompileOptions options;
	options.GenerateDebugInfo = true;
	const auto debug = loader.Load("3D.glsl", options);
	REQUIRE(
	    std::distance(
	        std::filesystem::directory_iterator(directory.GetPath()),
	        std::filesystem::directory_iterator{}) == 4);
}
