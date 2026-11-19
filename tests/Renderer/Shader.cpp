#include "rdpch.h"

#include "Rod/Renderer/Shader.h"

#include "../OpenGLFixture.h"
#include "../TemporaryDirectory.h"
#include "Rod/Renderer/Renderer.h"
#include "Rod/Renderer/ShaderCacheDatabase.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

#include <chrono>

namespace {
	void RequireLinkedShaders()
	{
		for (const auto* filename : {"Texture.glsl", "3D.glsl", "Shadow.glsl"})
		{
			CAPTURE(filename);
			const auto path = std::filesystem::path(ROD_TEST_SHADER_DIRECTORY) / filename;
			const auto shader = Rod::Shader::Create(path.string());
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
	Rod::Renderer::Init(directory.GetPath());
	RequireLinkedShaders();
	auto& cache = Rod::ShaderCacheDatabase::Get();
	cache.Flush();

	std::vector<std::pair<std::filesystem::path, std::filesystem::file_time_type>> cachedFiles;
	constexpr auto cacheAge = std::chrono::hours{24};
	const auto timestamp = std::filesystem::file_time_type::clock::now() - cacheAge;
	for (const auto& entry : std::filesystem::directory_iterator(cache.GetCacheDirectory()))
	{
		if (entry.path().filename() == "cache.yaml")
			continue;
		std::filesystem::last_write_time(entry.path(), timestamp);
		cachedFiles.emplace_back(entry.path(), std::filesystem::last_write_time(entry.path()));
	}
	constexpr size_t binaryCount = 6;
	REQUIRE(cachedFiles.size() == binaryCount);

	const Rod::ShaderCacheDatabase restored(cache.GetCacheDirectory());
	cache = restored;
	RequireLinkedShaders();
	for (const auto& [path, modified] : cachedFiles)
	{
		CAPTURE(path);
		// libc++ can store filesystem timestamps with a 128-bit tick count.
		REQUIRE((std::filesystem::last_write_time(path) == modified));
	}
	Rod::Renderer::Shutdown();
}
