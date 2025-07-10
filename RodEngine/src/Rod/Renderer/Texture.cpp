#include "rdpch.h"
#include "Texture.h"
#include "Renderer.h"

#include "Rod/Platform/OpenGL/OpenGLTexture.h"

namespace Rod {
	Ref<Texture2D> Rod::Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}

