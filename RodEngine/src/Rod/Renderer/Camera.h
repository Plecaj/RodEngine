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
		Camera& operator=(const Camera&) = default;
		Camera(Camera&&) = default;
		Camera& operator=(Camera&&) = default;

		[[nodiscard]] constexpr const glm::mat4& GetProjection() const noexcept { return m_Projection; }

	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};

}
