#pragma once

#include "Rod/Core/Core.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
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
	[[nodiscard]] constexpr auto ShaderDataTypeSize(ShaderDataType type) -> uint32_t
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

		constexpr BufferElement(ShaderDataType type, std::string name, bool normalized = false)
		    : Type(type), Name(std::move(name)), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{}

		BufferElement() : Type(ShaderDataType::None), Name({}), Size(0), Offset(0), Normalized(false) {};

		// NOLINTBEGIN(readability-magic-numbers)
		[[nodiscard]] constexpr auto GetComponentCount() const -> uint32_t
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
		[[nodiscard]] constexpr auto GetStride() const noexcept -> uint32_t { return m_Stride; };
		[[nodiscard]] auto GetElements() const noexcept -> std::span<const BufferElement> { return m_Elements; }

		[[nodiscard]] auto begin() noexcept -> std::vector<BufferElement>::iterator { return m_Elements.begin(); }
		[[nodiscard]] auto end() noexcept -> std::vector<BufferElement>::iterator { return m_Elements.end(); }
		[[nodiscard]] auto begin() const noexcept -> std::vector<BufferElement>::const_iterator
		{
			return m_Elements.begin();
		}
		[[nodiscard]] auto end() const noexcept -> std::vector<BufferElement>::const_iterator
		{
			return m_Elements.end();
		}

	private:
		void CalculateOffsetsAndStride() noexcept
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
		auto operator=(const VertexBuffer&) -> VertexBuffer& = delete;
		VertexBuffer(VertexBuffer&&) = delete;
		auto operator=(VertexBuffer&&) -> VertexBuffer& = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(std::span<const std::byte> data) = 0;

		virtual void SetBufferLayout(const BufferLayout& layout) = 0;
		[[nodiscard]] virtual auto GetLayout() const noexcept -> const BufferLayout& = 0;

		[[nodiscard]] static auto Create(uint32_t size) -> Scope<VertexBuffer>;
		[[nodiscard]] static auto Create(std::span<const std::byte> vertices) -> Scope<VertexBuffer>;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		virtual ~IndexBuffer() = default;
		IndexBuffer(const IndexBuffer&) = delete;
		auto operator=(const IndexBuffer&) -> IndexBuffer& = delete;
		IndexBuffer(IndexBuffer&&) = delete;
		auto operator=(IndexBuffer&&) -> IndexBuffer& = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		[[nodiscard]] virtual auto GetCount() const noexcept -> uint32_t = 0;

		[[nodiscard]] static auto Create(std::span<const uint32_t> indices) -> Scope<IndexBuffer>;
	};

	class UniformBuffer
	{
	public:
		UniformBuffer() = default;
		virtual ~UniformBuffer() = default;
		UniformBuffer(const UniformBuffer&) = delete;
		auto operator=(const UniformBuffer&) -> UniformBuffer& = delete;
		UniformBuffer(UniformBuffer&&) = delete;
		auto operator=(UniformBuffer&&) -> UniformBuffer& = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void BindBase(uint32_t bindingIndex) const = 0;

		virtual void SetData(std::span<const std::byte> data) = 0;

		[[nodiscard]] static auto Create(uint32_t size) -> Scope<UniformBuffer>;
	};
}
