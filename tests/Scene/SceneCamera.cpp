#include "rdpch.h"

#include "Rod/Scene/SceneCamera.h"

#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace {
	// NOLINTNEXTLINE(readability-function-size)
	void RequireProjectionNear(const glm::mat4& actual, const glm::mat4& expected)
	{
		for (glm::mat4::length_type column = 0; column < expected.length(); ++column)
		{
			for (glm::vec4::length_type row = 0; row < expected[column].length(); ++row)
			{
				CAPTURE(column, row);
				REQUIRE_THAT(actual[column][row], Catch::Matchers::WithinAbs(expected[column][row], 0.00001));
			}
		}
	}
}

TEST_CASE("Scene camera updates orthographic and perspective projections on resize", "[scene][camera]")
{
	constexpr float fov = glm::radians(60.0f);

	Rod::SceneCamera camera;
	camera.SetViewportSize(800, 400);
	camera.SetOrthographic(12.0f, -2.0f, 4.0f);
	RequireProjectionNear(camera.GetProjection(), glm::ortho(-12.0f, 12.0f, -6.0f, 6.0f, -2.0f, 4.0f));

	camera.SetPerspective(fov, 0.1f, 500.0f);
	RequireProjectionNear(camera.GetProjection(), glm::perspective(fov, 2.0f, 0.1f, 500.0f));

	camera.SetViewportSize(400, 400);
	RequireProjectionNear(camera.GetProjection(), glm::perspective(fov, 1.0f, 0.1f, 500.0f));
}

TEST_CASE("Scene camera ignores zero-sized viewport dimensions", "[scene][camera]")
{
	Rod::SceneCamera camera;
	camera.SetViewportSize(800, 400);
	const glm::mat4 projection = camera.GetProjection();
	camera.SetViewportSize(0, 400);
	RequireProjectionNear(camera.GetProjection(), projection);
	camera.SetViewportSize(800, 0);
	RequireProjectionNear(camera.GetProjection(), projection);
}

TEST_CASE("Scene resize preserves fixed-aspect camera projection", "[scene][camera]")
{
	Rod::Scene scene;
	scene.ResizeViewport(800, 400);
	auto fixedEntity = scene.CreateEntity("Fixed");
	auto& fixed = fixedEntity.AddComponent<Rod::CameraComponent>();
	fixed.FixedAspectRatio = true;
	const glm::mat4 fixedProjection = fixed.Camera.GetProjection();

	auto resizableEntity = scene.CreateEntity("Resizable");
	const auto& resizable = resizableEntity.AddComponent<Rod::CameraComponent>();
	scene.ResizeViewport(400, 400);

	RequireProjectionNear(fixed.Camera.GetProjection(), fixedProjection);
	RequireProjectionNear(resizable.Camera.GetProjection(), glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -1.0f, 1.0f));
}
