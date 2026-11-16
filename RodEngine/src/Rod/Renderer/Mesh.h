#pragma once
#include "Buffer.h"
#include "Material.h"
#include "VertexArray.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tiny_gltf.h>
#include <vector>

namespace Rod {
	class Mesh
	{
	public:
		explicit Mesh(const std::string& glbPath);

		[[nodiscard]] const VertexArray* GetVAO() const noexcept { return m_VAO.get(); }
		[[nodiscard]] const Material& GetMaterial() const noexcept { return m_Material; }
		[[nodiscard]] const std::string& GetPath() const noexcept { return m_SourcePath; }

	private:
		static constexpr size_t VECTOR_COMPONENT_COUNT = 3;
		static constexpr size_t COLOR_COMPONENT_COUNT = 4;
		static constexpr size_t VERTEX_COMPONENT_COUNT = VECTOR_COMPONENT_COUNT * 2;

		template <typename T>
		[[nodiscard]] static T ReadScalar(const uint8_t* data);
		[[nodiscard]] static const uint8_t*
		AccessorData(const tinygltf::Model& model, const tinygltf::Accessor& accessor, size_t elementSize);
		[[nodiscard]] static std::vector<float>
		ReadVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive);
		[[nodiscard]] static std::vector<uint32_t>
		ReadIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive);
		template <typename Vector>
		[[nodiscard]] static Vector ToFloatVector(const std::vector<double>& values);

		void ProcessPrimitive(std::vector<float>& vertices, std::vector<uint32_t>& indices);
		void ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive);

	private:
		Scope<VertexArray> m_VAO;
		Material m_Material;
		std::string m_SourcePath;
	};

}
