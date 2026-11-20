#pragma once

#include <glm/glm.hpp>

namespace Rod::Math {

	[[nodiscard]] auto DecomposeTransform(
	    const glm::mat4& transform,
	    glm::vec3& translation,
	    glm::vec3& rotation,
	    glm::vec3& scale) noexcept -> bool;
}
