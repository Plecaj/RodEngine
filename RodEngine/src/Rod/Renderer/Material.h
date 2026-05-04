#pragma once
#include <glm/glm.hpp>

#include "Rod/Core/Core.h"

namespace Rod {

	class Material {
	public:
		static Ref<Material> Create() { return CreateRef<Material>(); }

		const glm::vec4& GetAlbedo() const { return m_Albedo; }
		float GetRoughness() const { return m_Roughness; }
		float GetMetallic() const { return m_Metallic; }
		const glm::vec3& GetEmissive() const { return m_Emissive; }

		void SetAlbedo(const glm::vec4& albedo) { m_Albedo = albedo; }
		void SetRoughness(float roughness) { m_Roughness = roughness; }
		void SetMetallic(float metallic) { m_Metallic = metallic; }
		void SetEmissive(const glm::vec3& emissive) { m_Emissive = emissive; }

	private:
		glm::vec4 m_Albedo = glm::vec4(1.0f);
		float m_Roughness = 1.0f;
		float m_Metallic = 0.0f;
		glm::vec3 m_Emissive = glm::vec3(0.0f);
	};

}
