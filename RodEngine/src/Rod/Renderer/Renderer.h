#pragma once

#include "RenderCommand.h"

#include <filesystem>

namespace Rod {

	class Renderer
	{
	public:
		static void Init(const std::filesystem::path& shaderDirectory);
		[[nodiscard]] static const std::filesystem::path& GetShaderDirectory();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		[[nodiscard]] static RendererAPI::API GetAPI() noexcept { return RendererAPI::GetAPI(); }
	};

}
