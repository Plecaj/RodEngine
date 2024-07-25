#include "rdpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Rod {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();

}