#include "rdpch.h"

#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Rod::Math {

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		glm::quat orientation;
		glm::vec3 skew;
		glm::vec4 perspective;

		if (!glm::decompose(transform, scale, orientation, translation, skew, perspective))
			return false;

		glm::mat4 rotationMatrix = transform;
		constexpr auto translationColumn = 3;
		constexpr auto spatialAxes = 3;
		rotationMatrix[translationColumn] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		for (int i = 0; i < spatialAxes; ++i)
		{
			if (!glm::epsilonEqual(scale[i], 0.0f, glm::epsilon<float>()))
				rotationMatrix[i] /= scale[i];
		}

		glm::extractEulerAngleXYZ(rotationMatrix, rotation.x, rotation.y, rotation.z);

		return true;
	}

}
