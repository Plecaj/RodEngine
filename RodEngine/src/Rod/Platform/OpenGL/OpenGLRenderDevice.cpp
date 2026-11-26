#include "rdpch.h"

#include "OpenGLBuffer.h"
#include "OpenGLFrameBuffer.h"
#include "OpenGLTexture.h"
#include "OpenGLVertexArray.h"
#include "Rod/Renderer/RenderDevice.h"

#include <glad/glad.h>

#include <array>
#include <stdexcept>

namespace Rod {

	// NOLINTBEGIN(readability-convert-member-functions-to-static)
	void RenderDevice::ConfigureDefaults() const
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
	}

	void RenderDevice::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
	{
		glViewport(
		    static_cast<GLint>(x),
		    static_cast<GLint>(y),
		    static_cast<GLsizei>(width),
		    static_cast<GLsizei>(height));
	}

	void RenderDevice::Clear(const glm::vec4& color) const
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderDevice::ClearDepth() const
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void RenderDevice::DrawIndexed(const VertexArray& vertexArray) const
	{
		const auto indexBuffer = vertexArray.GetIndexBuffer();
		if (!indexBuffer)
			throw std::logic_error("Indexed draw requires an index buffer");
		DrawIndexed(vertexArray, indexBuffer->GetCount());
	}

	void RenderDevice::DrawIndexed(const VertexArray& vertexArray, uint32_t indexCount) const
	{
		vertexArray.Bind();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
	}

	auto RenderDevice::GetRenderTarget() const -> RenderTarget
	{
		GLint drawFramebuffer = 0;
		GLint readFramebuffer = 0;
		std::array<GLint, 4> viewport{};
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
		glGetIntegerv(GL_VIEWPORT, viewport.data());
		const auto [x, y, width, height] = viewport;
		return {
		    .DrawFramebuffer = static_cast<uint32_t>(drawFramebuffer),
		    .ReadFramebuffer = static_cast<uint32_t>(readFramebuffer),
		    .X = x,
		    .Y = y,
		    .Width = static_cast<uint32_t>(width),
		    .Height = static_cast<uint32_t>(height),
		};
	}

	void RenderDevice::SetRenderTarget(const RenderTarget& target) const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.DrawFramebuffer);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, target.ReadFramebuffer);
		glViewport(target.X, target.Y, static_cast<GLsizei>(target.Width), static_cast<GLsizei>(target.Height));
	}

	auto RenderDevice::CreateFramebuffer(const FramebufferSpecification& spec) const -> Scope<Framebuffer>
	{
		return CreateScope<OpenGLFramebuffer>(spec);
	}

	auto RenderDevice::CreateVertexArray() const -> Scope<VertexArray>
	{
		return CreateScope<OpenGLVertexArray>();
	}

	auto RenderDevice::CreateVertexBuffer(uint32_t size) const -> Scope<VertexBuffer>
	{
		return CreateScope<OpenGLVertexBuffer>(size);
	}

	auto RenderDevice::CreateVertexBuffer(std::span<const std::byte> vertices) const -> Scope<VertexBuffer>
	{
		return CreateScope<OpenGLVertexBuffer>(vertices);
	}

	auto RenderDevice::CreateIndexBuffer(std::span<const uint32_t> indices) const -> Scope<IndexBuffer>
	{
		return CreateScope<OpenGLIndexBuffer>(indices);
	}

	auto RenderDevice::CreateUniformBuffer(uint32_t size) const -> Scope<UniformBuffer>
	{
		return CreateScope<OpenGLUniformBuffer>(size);
	}

	auto RenderDevice::CreateTexture2D(uint32_t width, uint32_t height) const -> Scope<Texture2D>
	{
		return CreateScope<OpenGLTexture2D>(width, height);
	}

	auto RenderDevice::LoadTexture2D(const std::string& path) const -> Scope<Texture2D>
	{
		return CreateScope<OpenGLTexture2D>(path);
	}
	// NOLINTEND(readability-convert-member-functions-to-static)

}
