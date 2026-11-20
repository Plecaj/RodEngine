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
		RD_PROFILE_FUNCTION();

		g_ShaderDirectory = std::filesystem::absolute(shaderDirectory).lexically_normal();
		RenderCommand::Init();
	}

	void Renderer::Shutdown()
	{
		RD_PROFILE_FUNCTION();
		ShaderCacheDatabase::Get().Flush();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::SetViewport(0, 0, width, height);
	}

}
