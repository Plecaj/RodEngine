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

	REQUIRE(elements.size() == 3);
	REQUIRE(elements[0].Offset == 0);
	REQUIRE(elements[1].Offset == 12);
	REQUIRE(elements[2].Offset == 20);
	REQUIRE(layout.GetStride() == 24);
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
	    {.Type = Rod::ShaderDataType::Float, .Size = 4, .Components = 1},
	    {.Type = Rod::ShaderDataType::Float2, .Size = 8, .Components = 2},
	    {.Type = Rod::ShaderDataType::Float3, .Size = 12, .Components = 3},
	    {.Type = Rod::ShaderDataType::Float4, .Size = 16, .Components = 4},
	    {.Type = Rod::ShaderDataType::Mat3, .Size = 36, .Components = 9},
	    {.Type = Rod::ShaderDataType::Mat4, .Size = 64, .Components = 16},
	    {.Type = Rod::ShaderDataType::Int, .Size = 4, .Components = 1},
	    {.Type = Rod::ShaderDataType::Int2, .Size = 8, .Components = 2},
	    {.Type = Rod::ShaderDataType::Int3, .Size = 12, .Components = 3},
	    {.Type = Rod::ShaderDataType::Int4, .Size = 16, .Components = 4},
	    {.Type = Rod::ShaderDataType::Bool, .Size = 1, .Components = 1},
	});

	for (const auto& type : cases)
	{
		CAPTURE(type.Type);
		const Rod::BufferElement element(type.Type, "Attribute");
		REQUIRE(element.Size == type.Size);
		REQUIRE(element.GetComponentCount() == type.Components);
	}
}
