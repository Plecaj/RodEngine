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
		~OpenGLVertexBuffer() override;
		OpenGLVertexBuffer(const OpenGLVertexBuffer&) = delete;
		auto operator=(const OpenGLVertexBuffer&) -> OpenGLVertexBuffer& = delete;
		OpenGLVertexBuffer(OpenGLVertexBuffer&&) = delete;
		auto operator=(OpenGLVertexBuffer&&) -> OpenGLVertexBuffer& = delete;

		void Bind() const override;
		void Unbind() const override;

		void SetData(std::span<const std::byte> data) override;

		[[nodiscard]] auto GetLayout() const noexcept -> const BufferLayout& override { return m_Layout; };
		void SetBufferLayout(const BufferLayout& layout) override { m_Layout = layout; };

	private:
		uint32_t m_RendererID = 0;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		explicit OpenGLIndexBuffer(std::span<const uint32_t> indices);
		~OpenGLIndexBuffer() override;
		OpenGLIndexBuffer(const OpenGLIndexBuffer&) = delete;
		auto operator=(const OpenGLIndexBuffer&) -> OpenGLIndexBuffer& = delete;
		OpenGLIndexBuffer(OpenGLIndexBuffer&&) = delete;
		auto operator=(OpenGLIndexBuffer&&) -> OpenGLIndexBuffer& = delete;

		[[nodiscard]] auto GetCount() const noexcept -> uint32_t override { return m_Count; }

		void Bind() const override;
		void Unbind() const override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Count = 0;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		explicit OpenGLUniformBuffer(uint32_t size);
		~OpenGLUniformBuffer() override;
		OpenGLUniformBuffer(const OpenGLUniformBuffer&) = delete;
		auto operator=(const OpenGLUniformBuffer&) -> OpenGLUniformBuffer& = delete;
		OpenGLUniformBuffer(OpenGLUniformBuffer&&) = delete;
		auto operator=(OpenGLUniformBuffer&&) -> OpenGLUniformBuffer& = delete;

		void Bind() const override;
		void Unbind() const override;
		void BindBase(uint32_t bindingIndex) const override;

		void SetData(std::span<const std::byte> data) override;

	private:
		uint32_t m_RendererID = 0;
	};

}
