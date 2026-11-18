#include "rdpch.h"

#include "Rod/Renderer/Buffer.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Buffer layouts pack vertex attributes", "[renderer][buffer]")
{
	const Rod::BufferLayout layout = {
	    {Rod::ShaderDataType::Float3, "Position"},
	    {Rod::ShaderDataType::Float2, "UV"},
	    {Rod::ShaderDataType::Int, "Entity"}};
	const auto elements = layout.GetElements();
	constexpr uint32_t positionSize = 12;
	constexpr uint32_t uvSize = 8;
	constexpr uint32_t entitySize = 4;
	constexpr size_t attributeCount = 3;

	REQUIRE(elements.size() == attributeCount);
	REQUIRE(elements[0].Offset == 0);
	REQUIRE(elements[1].Offset == positionSize);
	REQUIRE(elements[2].Offset == positionSize + uvSize);
	REQUIRE(layout.GetStride() == positionSize + uvSize + entitySize);
}

TEST_CASE("Buffer elements describe shader types", "[renderer][buffer]")
{
	struct TypeCase
	{
		Rod::ShaderDataType Type;
		uint32_t Size;
		uint32_t Components;
	};
	constexpr auto cases = std::to_array<TypeCase>({
	    {Rod::ShaderDataType::Float, 4, 1},
	    {Rod::ShaderDataType::Float2, 8, 2},
	    {Rod::ShaderDataType::Float3, 12, 3},
	    {Rod::ShaderDataType::Float4, 16, 4},
	    {Rod::ShaderDataType::Mat3, 36, 9},
	    {Rod::ShaderDataType::Mat4, 64, 16},
	    {Rod::ShaderDataType::Int, 4, 1},
	    {Rod::ShaderDataType::Int2, 8, 2},
	    {Rod::ShaderDataType::Int3, 12, 3},
	    {Rod::ShaderDataType::Int4, 16, 4},
	    {Rod::ShaderDataType::Bool, 1, 1},
	});

	for (const auto& type : cases)
	{
		CAPTURE(type.Type);
		const Rod::BufferElement element(type.Type, "Attribute");
		REQUIRE(element.Size == type.Size);
		REQUIRE(element.GetComponentCount() == type.Components);
	}
}
