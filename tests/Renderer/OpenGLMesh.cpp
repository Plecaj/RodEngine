#include "rdpch.h"

#include "../OpenGLFixture.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Renderer/VertexArray.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Meshes load vertices, indices and materials", "[renderer][opengl]")
{
	std::string filename;
	float normalZ = 0.0f;
	Rod::Material expectedMaterial;
	SECTION("with normals and material")
	{
		filename = "triangle.glb";
		normalZ = 1.0f;
		constexpr glm::vec4 albedo{0.25f, 0.5f, 0.75f, 1.0f};
		constexpr glm::vec3 emissive{0.125f, 0.25f, 0.5f};
		constexpr float roughness = 0.5f;
		constexpr float metallic = 0.25f;
		expectedMaterial.SetAlbedo(albedo);
		expectedMaterial.SetEmissive(emissive);
		expectedMaterial.SetRoughness(roughness);
		expectedMaterial.SetMetallic(metallic);
	}
	SECTION("without normals and material")
	{
		filename = "triangle-bare.glb";
	}

	const auto path = AssetPath(filename).string();
	const Rod::Mesh mesh(path);
	const auto vao = mesh.GetVAO();
	REQUIRE(vao.has_value());
	REQUIRE(vao->GetVertexBuffers().size() == 1);
	const auto expectedVertices =
	    std::to_array<float>({0, 0, 0, 0, 0, normalZ, 1, 0, 0, 0, 0, normalZ, 0, 1, 0, 0, 0, normalZ});
	std::array<float, expectedVertices.size()> vertices{};
	vao->GetVertexBuffers().front()->Bind();
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices.data());
	REQUIRE(vertices == expectedVertices);

	constexpr auto expectedIndices = std::to_array<uint32_t>({0, 1, 2});
	std::array<uint32_t, expectedIndices.size()> indices{};
	vao->Bind();
	const auto indexBuffer = vao->GetIndexBuffer();
	REQUIRE(indexBuffer.has_value());
	REQUIRE(indexBuffer->GetCount() == expectedIndices.size());
	indexBuffer->Bind();
	glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices.data());
	REQUIRE(indices == expectedIndices);
	REQUIRE(mesh.GetMaterial().GetAlbedo() == expectedMaterial.GetAlbedo());
	REQUIRE(mesh.GetMaterial().GetEmissive() == expectedMaterial.GetEmissive());
	REQUIRE(mesh.GetMaterial().GetRoughness() == expectedMaterial.GetRoughness());
	REQUIRE(mesh.GetMaterial().GetMetallic() == expectedMaterial.GetMetallic());
	vao->Unbind();
	REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Vertex arrays can have no index buffer", "[renderer][opengl]")
{
	const auto vertexArray = Rod::VertexArray::Create();
	REQUIRE_FALSE(vertexArray->GetIndexBuffer().has_value());
}
