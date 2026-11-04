#pragma once

#include "Rod/Core/Core.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace Rod {

	enum class ShaderDataType : std::uint8_t
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Int,
		Int2,
		Int3,
		Int4,
		Bool
	};

	// NOLINTBEGIN(readability-magic-numbers)
	[[nodiscard]] static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:
				return 4;
			case ShaderDataType::Float2:
				return 4 * 2;
			case ShaderDataType::Float3:
				return 4 * 3;
			case ShaderDataType::Float4:
				return 4 * 4;
			case ShaderDataType::Mat3:
				return 4 * 3 * 3;
			case ShaderDataType::Mat4:
				return 4 * 4 * 4;
			case ShaderDataType::Int:
				return 4;
			case ShaderDataType::Int2:
				return 4 * 2;
			case ShaderDataType::Int3:
				return 4 * 3;
			case ShaderDataType::Int4:
				return 4 * 4;
			case ShaderDataType::Bool:
				return 1;
			case ShaderDataType::None:
				break;
		}

		RD_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}
	// NOLINTEND(readability-magic-numbers)

	struct BufferElement
	{
		ShaderDataType Type;
		std::string Name;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement(ShaderDataType type, std::string name, bool normalized = false)
		    : Type(type), Name(std::move(name)), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{}

		BufferElement() : Type(ShaderDataType::None), Name({}), Size(0), Offset(0), Normalized(false) {};

		// NOLINTBEGIN(readability-magic-numbers)
		[[nodiscard]] uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:
					return 1;
				case ShaderDataType::Float2:
					return 2;
				case ShaderDataType::Float3:
					return 3;
				case ShaderDataType::Float4:
					return 4;
				case ShaderDataType::Mat3:
					return 3 * 3;
				case ShaderDataType::Mat4:
					return 4 * 4;
				case ShaderDataType::Int:
					return 1;
				case ShaderDataType::Int2:
					return 2;
				case ShaderDataType::Int3:
					return 3;
				case ShaderDataType::Int4:
					return 4;
				case ShaderDataType::Bool:
					return 1;
				case ShaderDataType::None:
					break;
			}

			RD_CORE_ASSERT(false, "Unknown ShaderDataType");
			return 0;
		}
		// NOLINTEND(readability-magic-numbers)
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}
		[[nodiscard]] uint32_t GetStride() const noexcept { return m_Stride; };
		[[nodiscard]] const std::vector<BufferElement>& GetElements() const noexcept { return m_Elements; }

		[[nodiscard]] std::vector<BufferElement>::iterator begin() noexcept { return m_Elements.begin(); }
		[[nodiscard]] std::vector<BufferElement>::iterator end() noexcept { return m_Elements.end(); }
		[[nodiscard]] std::vector<BufferElement>::const_iterator begin() const noexcept { return m_Elements.begin(); }
		[[nodiscard]] std::vector<BufferElement>::const_iterator end() const noexcept { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		virtual ~VertexBuffer() = default;
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual void SetBufferLayout(const BufferLayout& layout) = 0;
		[[nodiscard]] virtual const BufferLayout& GetLayout() const noexcept = 0;

		[[nodiscard]] static Scope<VertexBuffer> Create(uint32_t size);
		[[nodiscard]] static Scope<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		virtual ~IndexBuffer() = default;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer(IndexBuffer&&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		[[nodiscard]] virtual uint32_t GetCount() const noexcept = 0;

		[[nodiscard]] static Scope<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};

	class UniformBuffer
	{
	public:
		UniformBuffer() = default;
		virtual ~UniformBuffer() = default;
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void BindBase(uint32_t bindingIndex) const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		[[nodiscard]] static Scope<UniformBuffer> Create(uint32_t size);
	};
}
