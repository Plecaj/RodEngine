#include "rdpch.h"
#include "Renderer.h"

namespace Rod {

	void Renderer::Init()
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::Init();
	}

	void Renderer::Shutdown()
	{
		RD_PROFILE_FUNCTION();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::SetViewport(0, 0, width, height);
	}

}
