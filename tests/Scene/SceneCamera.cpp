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
		constexpr double tolerance = 0.00001;
		for (glm::mat4::length_type column = 0; column < expected.length(); ++column)
		{
			for (glm::vec4::length_type row = 0; row < expected[column].length(); ++row)
			{
				CAPTURE(column, row);
				REQUIRE_THAT(actual[column][row], Catch::Matchers::WithinAbs(expected[column][row], tolerance));
			}
		}
	}
}

TEST_CASE("Scene camera updates orthographic and perspective projections on resize", "[scene][camera]")
{
	constexpr uint32_t wideWidth = 800;
	constexpr uint32_t wideHeight = 400;
	constexpr uint32_t squareSize = 400;
	constexpr float orthographicSize = 12.0f;
	constexpr float orthographicHalfHeight = 6.0f;
	constexpr float orthographicNearClip = -2.0f;
	constexpr float orthographicFarClip = 4.0f;
	constexpr float perspectiveNearClip = 0.1f;
	constexpr float perspectiveFarClip = 500.0f;
	constexpr float wideAspectRatio = 2.0f;
	constexpr float fov = glm::radians(60.0f);

	Rod::SceneCamera camera;
	camera.SetViewportSize(wideWidth, wideHeight);
	camera.SetOrthographic(orthographicSize, orthographicNearClip, orthographicFarClip);
	RequireProjectionNear(
	    camera.GetProjection(),
	    glm::ortho(
	        -orthographicSize,
	        orthographicSize,
	        -orthographicHalfHeight,
	        orthographicHalfHeight,
	        orthographicNearClip,
	        orthographicFarClip));

	camera.SetPerspective(fov, perspectiveNearClip, perspectiveFarClip);
	RequireProjectionNear(
	    camera.GetProjection(),
	    glm::perspective(fov, wideAspectRatio, perspectiveNearClip, perspectiveFarClip));

	camera.SetViewportSize(squareSize, squareSize);
	RequireProjectionNear(camera.GetProjection(), glm::perspective(fov, 1.0f, perspectiveNearClip, perspectiveFarClip));
}

TEST_CASE("Scene camera ignores zero-sized viewport dimensions", "[scene][camera]")
{
	constexpr uint32_t viewportWidth = 800;
	constexpr uint32_t viewportHeight = 400;

	Rod::SceneCamera camera;
	camera.SetViewportSize(viewportWidth, viewportHeight);
	const glm::mat4 projection = camera.GetProjection();
	camera.SetViewportSize(0, viewportHeight);
	RequireProjectionNear(camera.GetProjection(), projection);
	camera.SetViewportSize(viewportWidth, 0);
	RequireProjectionNear(camera.GetProjection(), projection);
}

TEST_CASE("Scene resize preserves fixed-aspect camera projection", "[scene][camera]")
{
	constexpr uint32_t wideWidth = 800;
	constexpr uint32_t wideHeight = 400;
	constexpr uint32_t squareSize = 400;
	constexpr float defaultHalfExtent = 5.0f;

	Rod::Scene scene;
	scene.OnViewportResize(wideWidth, wideHeight);
	auto fixedEntity = scene.CreateEntity("Fixed");
	auto& fixed = fixedEntity.AddComponent<Rod::CameraComponent>();
	fixed.FixedAspectRatio = true;
	const glm::mat4 fixedProjection = fixed.Camera.GetProjection();

	auto resizableEntity = scene.CreateEntity("Resizable");
	auto& resizable = resizableEntity.AddComponent<Rod::CameraComponent>();
	scene.OnViewportResize(squareSize, squareSize);

	RequireProjectionNear(fixed.Camera.GetProjection(), fixedProjection);
	RequireProjectionNear(
	    resizable.Camera.GetProjection(),
	    glm::ortho(-defaultHalfExtent, defaultHalfExtent, -defaultHalfExtent, defaultHalfExtent, -1.0f, 1.0f));
}
