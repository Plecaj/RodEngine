#include "rdpch.h"

#include "RenderCommand.h"

#include "Rod/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Rod {

	auto RenderCommand::GetRendererAPI() -> RendererAPI&
	{
		static OpenGLRendererAPI s_RendererAPI;
		return s_RendererAPI;
	}

}
