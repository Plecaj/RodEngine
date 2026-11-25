#include "rdpch.h"

#include "Rod/Renderer/Mesh.h"

#include "../TemporaryDirectory.h"
#include "Rod/Renderer/RenderDevice.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <stdexcept>

TEST_CASE("Mesh construction rejects missing or corrupt files", "[renderer][loading]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const auto filepath = (directory.GetPath() / "mesh.glb").string();
	SECTION("missing file")
	{}
	SECTION("corrupt file")
	{
		std::ofstream file(filepath);
		file << "not a GLB";
	}
	REQUIRE_THROWS_AS(Rod::Mesh(Rod::RenderDevice{}, filepath), std::runtime_error);
}
