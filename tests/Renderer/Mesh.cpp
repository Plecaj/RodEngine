#include "rdpch.h"

#include "Rod/Core/UUID.h"
#include "Rod/Renderer/Mesh.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <stdexcept>
#include <system_error>

namespace {
	class MeshLoadingFixture
	{
	public:
		MeshLoadingFixture()
		{
			do
			{
				m_Directory = std::filesystem::temp_directory_path() /
				              ("rod-mesh-loading-" + std::to_string(static_cast<uint64_t>(Rod::UUID{})));
			} while (!std::filesystem::create_directory(m_Directory));
		}
		MeshLoadingFixture(const MeshLoadingFixture&) = delete;
		MeshLoadingFixture& operator=(const MeshLoadingFixture&) = delete;
		MeshLoadingFixture(MeshLoadingFixture&&) = delete;
		MeshLoadingFixture& operator=(MeshLoadingFixture&&) = delete;
		~MeshLoadingFixture()
		{
			std::error_code error;
			std::filesystem::remove_all(m_Directory, error);
		}

	protected:
		std::string Filepath() const { return (m_Directory / "mesh.glb").string(); }

	private:
		std::filesystem::path m_Directory;
	};
}

TEST_CASE_METHOD(MeshLoadingFixture, "Mesh construction rejects missing or corrupt files", "[renderer][loading]")
{
	SECTION("missing file") {}
	SECTION("corrupt file")
	{
		std::ofstream file(Filepath());
		file << "not a GLB";
	}
	REQUIRE_THROWS_AS(Rod::Mesh(Filepath()), std::runtime_error);
}
