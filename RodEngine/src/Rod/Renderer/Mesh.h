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

		[[nodiscard]] auto GetVAO() const noexcept -> std::optional<const VertexArray&>
		{
			if (m_VAO)
				return *m_VAO;
			return std::nullopt;
		}
		[[nodiscard]] auto GetMaterial() const noexcept -> const Material& { return m_Material; }
		[[nodiscard]] auto GetPath() const noexcept -> const std::string& { return m_SourcePath; }

	private:
		void ProcessPrimitive(std::span<const float> vertices, std::span<const uint32_t> indices);
		void ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive);

	private:
		Scope<VertexArray> m_VAO;
		Material m_Material;
		std::string m_SourcePath;
	};

}
