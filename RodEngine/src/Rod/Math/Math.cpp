#include "rdpch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Rod::Math {

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		glm::quat orientation;
		glm::vec3 skew;
		glm::vec4 perspective;

		if (!glm::decompose(transform, scale, orientation, translation, skew, perspective))
			return false;

		rotation = glm::eulerAngles(orientation);
		return true;
	}

}
