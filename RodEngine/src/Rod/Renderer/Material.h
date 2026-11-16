#pragma once

#include <glm/glm.hpp>

namespace Rod {

	class Material
	{
	public:
		[[nodiscard]] constexpr const glm::vec4& GetAlbedo() const noexcept { return m_Albedo; }
		[[nodiscard]] constexpr float GetRoughness() const noexcept { return m_Roughness; }
		[[nodiscard]] constexpr float GetMetallic() const noexcept { return m_Metallic; }
		[[nodiscard]] constexpr const glm::vec3& GetEmissive() const noexcept { return m_Emissive; }

		constexpr void SetAlbedo(const glm::vec4& albedo) noexcept { m_Albedo = albedo; }
		constexpr void SetRoughness(float roughness) noexcept { m_Roughness = roughness; }
		constexpr void SetMetallic(float metallic) noexcept { m_Metallic = metallic; }
		constexpr void SetEmissive(const glm::vec3& emissive) noexcept { m_Emissive = emissive; }

	private:
		glm::vec4 m_Albedo = glm::vec4(1.0f);
		float m_Roughness = 1.0f;
		float m_Metallic = 0.0f;
		glm::vec3 m_Emissive = glm::vec3(0.0f);
	};

}
