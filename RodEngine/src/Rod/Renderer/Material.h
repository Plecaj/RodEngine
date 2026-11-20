#pragma once

#include <glm/glm.hpp>

namespace Rod {

	class Material
	{
	public:
		[[nodiscard]] constexpr auto GetAlbedo() const noexcept -> const glm::vec4& { return m_Albedo; }
		[[nodiscard]] constexpr auto GetRoughness() const noexcept -> float { return m_Roughness; }
		[[nodiscard]] constexpr auto GetMetallic() const noexcept -> float { return m_Metallic; }
		[[nodiscard]] constexpr auto GetEmissive() const noexcept -> const glm::vec3& { return m_Emissive; }

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
