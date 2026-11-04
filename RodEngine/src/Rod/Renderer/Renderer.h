#pragma once

#include "RenderCommand.h"

namespace Rod {

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		[[nodiscard]] static RendererAPI::API GetAPI() noexcept { return RendererAPI::GetAPI(); }
	};

}
