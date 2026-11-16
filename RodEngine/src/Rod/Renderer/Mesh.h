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
		void ProcessPrimitive(std::vector<float>& vertices, std::vector<uint32_t>& indices);
		void ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive);

	private:
		Scope<VertexArray> m_VAO;
		Material m_Material;
		std::string m_SourcePath;
	};

}
