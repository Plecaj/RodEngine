#include "rdpch.h"

#include "Renderer.h"

#include "ShaderCacheDatabase.h"

namespace Rod {

	namespace {
		std::filesystem::path g_ShaderDirectory;
	}

	auto Renderer::GetShaderDirectory() noexcept -> const std::filesystem::path&
	{
		return g_ShaderDirectory;
	}

	void Renderer::Init(const std::filesystem::path& shaderDirectory)
	{
		g_ShaderDirectory = std::filesystem::absolute(shaderDirectory).lexically_normal();
		RenderCommand::Init();
	}

	void Renderer::Shutdown()
	{
		ShaderCacheDatabase::Get().Flush();
	}

}
