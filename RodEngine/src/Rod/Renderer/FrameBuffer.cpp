#include "rdpch.h"
#include "FrameBuffer.h"

#include "Rod/Renderer/RendererAPI.h"
#include "Rod/Platform/OpenGL/OpenGLFrameBuffer.h"

namespace Rod{

	Scope<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateScope<OpenGLFramebuffer>(spec);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
