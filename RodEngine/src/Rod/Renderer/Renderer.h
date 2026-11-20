#pragma once

#include "RenderCommand.h"

#include <filesystem>

namespace Rod {

	class Renderer
	{
	public:
		static void Init(const std::filesystem::path& shaderDirectory);
		[[nodiscard]] static auto GetShaderDirectory() noexcept -> const std::filesystem::path&;
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		[[nodiscard]] static auto GetAPI() noexcept -> RendererAPI::API { return RendererAPI::GetAPI(); }
	};

}
