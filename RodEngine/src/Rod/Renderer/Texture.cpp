#include "rdpch.h"
#include "Texture.h"
#include "Renderer.h"

#include "Rod/Platform/OpenGL/OpenGLTexture.h"

namespace Rod {
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(width, height);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<Texture2D> Rod::Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(path);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}

