#pragma once
#include "Buffer.h"
#include "Material.h"
#include "VertexArray.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>

namespace tinygltf {
	class Model;
	struct Primitive;
}

namespace Rod {
	class Mesh
	{
	public:
		explicit Mesh(const std::string& glbPath);

		[[nodiscard]] std::optional<const VertexArray&> GetVAO() const noexcept
		{
			if (m_VAO)
				return *m_VAO;
			return std::nullopt;
		}
		[[nodiscard]] const Material& GetMaterial() const noexcept { return m_Material; }
		[[nodiscard]] const std::string& GetPath() const noexcept { return m_SourcePath; }

	private:
		void ProcessPrimitive(std::span<const float> vertices, std::span<const uint32_t> indices);
		void ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive);

	private:
		Scope<VertexArray> m_VAO;
		Material m_Material;
		std::string m_SourcePath;
	};

}
