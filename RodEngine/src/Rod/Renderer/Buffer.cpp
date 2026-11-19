#include "rdpch.h"

#include "Buffer.h"

#include "RendererAPI.h"
#include "Rod/Platform/OpenGL/OpenGLBuffer.h"

namespace Rod {

	auto VertexBuffer::Create(uint32_t size) -> Scope<VertexBuffer>
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLVertexBuffer>(size);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	auto VertexBuffer::Create(std::span<const std::byte> vertices) -> Scope<VertexBuffer>
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLVertexBuffer>(vertices);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	};

	auto IndexBuffer::Create(std::span<const uint32_t> indices) -> Scope<IndexBuffer>
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLIndexBuffer>(indices);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	auto UniformBuffer::Create(uint32_t size) -> Scope<UniformBuffer>
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				RD_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLUniformBuffer>(size);
		}

		RD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
