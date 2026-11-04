#pragma once

#include <glm/glm.hpp>

namespace Rod {

	class Material
	{
	public:
		[[nodiscard]] const glm::vec4& GetAlbedo() const noexcept { return m_Albedo; }
		[[nodiscard]] float GetRoughness() const noexcept { return m_Roughness; }
		[[nodiscard]] float GetMetallic() const noexcept { return m_Metallic; }
		[[nodiscard]] const glm::vec3& GetEmissive() const noexcept { return m_Emissive; }

		void SetAlbedo(const glm::vec4& albedo) noexcept { m_Albedo = albedo; }
		void SetRoughness(float roughness) noexcept { m_Roughness = roughness; }
		void SetMetallic(float metallic) noexcept { m_Metallic = metallic; }
		void SetEmissive(const glm::vec3& emissive) noexcept { m_Emissive = emissive; }

	private:
		glm::vec4 m_Albedo = glm::vec4(1.0f);
		float m_Roughness = 1.0f;
		float m_Metallic = 0.0f;
		glm::vec3 m_Emissive = glm::vec3(0.0f);
	};

}
