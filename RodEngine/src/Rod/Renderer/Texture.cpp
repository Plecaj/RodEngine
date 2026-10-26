#include "rdpch.h"
#include "Texture.h"
#include "RendererAPI.h"

#include "Rod/Platform/OpenGL/OpenGLTexture.h"

namespace Rod {
	Scope<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateScope<OpenGLTexture2D>(width, height);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Scope<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateScope<OpenGLTexture2D>(path);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}

