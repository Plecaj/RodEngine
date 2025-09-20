#include "rdpch.h"
#include "Renderer.h"
#include "Renderer2D.h"

#include "Rod/Platform/OpenGL/OpenGLShader.h"

namespace Rod {

	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

}