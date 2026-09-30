#include "rdpch.h"
#include "RenderCommand.h"

#include "Rod/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Rod {

	Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<OpenGLRendererAPI>();

}
