#include "rdpch.h"

#include "FrameBuffer.h"

#include "Rod/Platform/OpenGL/OpenGLFrameBuffer.h"
#include "Rod/Renderer/RendererAPI.h"

namespace Rod {

	auto Framebuffer::Create(const FramebufferSpecification& spec) -> Scope<Framebuffer>
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLFramebuffer>(spec);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
