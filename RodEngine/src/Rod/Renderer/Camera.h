#pragma once

#include <glm/glm.hpp>

namespace Rod {

	class Camera
	{
	public:
		Camera() = default;
		constexpr explicit Camera(const glm::mat4& projection) noexcept : m_Projection(projection) {};

		virtual ~Camera() = default;
		Camera(const Camera&) = default;
		auto operator=(const Camera&) -> Camera& = default;
		Camera(Camera&&) = default;
		auto operator=(Camera&&) -> Camera& = default;

		[[nodiscard]] constexpr auto GetProjection() const noexcept -> const glm::mat4& { return m_Projection; }

	protected:
		constexpr void SetProjection(const glm::mat4& projection) noexcept { m_Projection = projection; }

	private:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};

}
