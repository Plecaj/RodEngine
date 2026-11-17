#pragma once

#include "Rod/Renderer/Buffer.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace Rod {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		explicit OpenGLVertexBuffer(uint32_t size);
		explicit OpenGLVertexBuffer(std::span<const std::byte> vertices);
		virtual ~OpenGLVertexBuffer() override;
		OpenGLVertexBuffer(const OpenGLVertexBuffer&) = delete;
		OpenGLVertexBuffer& operator=(const OpenGLVertexBuffer&) = delete;
		OpenGLVertexBuffer(OpenGLVertexBuffer&&) = delete;
		OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&&) = delete;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(std::span<const std::byte> data) override;

		[[nodiscard]] virtual const BufferLayout& GetLayout() const noexcept override { return m_Layout; };
		virtual void SetBufferLayout(const BufferLayout& layout) override { m_Layout = layout; };

	private:
		uint32_t m_RendererID = 0;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		explicit OpenGLIndexBuffer(std::span<const uint32_t> indices);
		virtual ~OpenGLIndexBuffer() override;
		OpenGLIndexBuffer(const OpenGLIndexBuffer&) = delete;
		OpenGLIndexBuffer& operator=(const OpenGLIndexBuffer&) = delete;
		OpenGLIndexBuffer(OpenGLIndexBuffer&&) = delete;
		OpenGLIndexBuffer& operator=(OpenGLIndexBuffer&&) = delete;

		[[nodiscard]] virtual uint32_t GetCount() const noexcept override { return m_Count; }

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Count = 0;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		explicit OpenGLUniformBuffer(uint32_t size);
		virtual ~OpenGLUniformBuffer() override;
		OpenGLUniformBuffer(const OpenGLUniformBuffer&) = delete;
		OpenGLUniformBuffer& operator=(const OpenGLUniformBuffer&) = delete;
		OpenGLUniformBuffer(OpenGLUniformBuffer&&) = delete;
		OpenGLUniformBuffer& operator=(OpenGLUniformBuffer&&) = delete;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindBase(uint32_t bindingIndex) const override;

		virtual void SetData(std::span<const std::byte> data) override;

	private:
		uint32_t m_RendererID = 0;
	};

}
