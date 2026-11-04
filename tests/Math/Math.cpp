#include "rdpch.h"

#include "Rod/Math/Math.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace {
	class TransformFixture
	{
	protected:
		static void RequireMatrixNear(const glm::mat4& actual, const glm::mat4& expected)
		{
			for (glm::mat4::length_type column = 0; column < expected.length(); ++column)
			{
				CAPTURE(column);
				RequireColumnNear(actual[column], expected[column]);
			}
		}

	private:
		static void RequireColumnNear(const glm::vec4& actual, const glm::vec4& expected)
		{
			constexpr double tolerance = 0.0001;
			for (glm::vec4::length_type row = 0; row < expected.length(); ++row)
			{
				CAPTURE(row);
				REQUIRE_THAT(actual[row], Catch::Matchers::WithinAbs(expected[row], tolerance));
			}
		}
	};
}

TEST_CASE_METHOD(TransformFixture, "Transform decomposition keeps translation, rotation and scale", "[math]")
{
	const glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -3.0f, 4.0f)) *
	                            glm::eulerAngleXYZ(0.3f, -0.4f, 0.5f) *
	                            glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 4.0f));

	glm::vec3 translation, rotation, scale;
	REQUIRE(Rod::Math::DecomposeTransform(transform, translation, rotation, scale));

	const glm::mat4 reconstructed = glm::translate(glm::mat4(1.0f), translation) *
	                                glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z) *
	                                glm::scale(glm::mat4(1.0f), scale);

	RequireMatrixNear(reconstructed, transform);
}

TEST_CASE("Transform decomposition rejects a singular transform", "[math]")
{
	const glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 1.0f));

	glm::vec3 translation, rotation, scale;
	REQUIRE(!Rod::Math::DecomposeTransform(transform, translation, rotation, scale));
}
