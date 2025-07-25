#include "rdpch.h"
#include "FrameBuffer.h"

#include "Rod/Renderer/Renderer.h"
#include "Rod/Platform/OpenGL/OpenGLFrameBuffer.h"

namespace Rod{

	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLFrameBuffer>(spec);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}