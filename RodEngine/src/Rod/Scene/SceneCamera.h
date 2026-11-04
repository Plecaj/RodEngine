#pragma once

#include "Rod/Renderer/Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cstdint>

namespace Rod {

	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType : uint8_t
		{
			Perspective = 0,
			Orthographic = 1
		};

	public:
		SceneCamera();

		void SetViewportSize(uint32_t width, uint32_t height);

		void SetProjectionType(ProjectionType type)
		{
			m_ProjectionType = type;
			RecalculateProjection();
		}
		[[nodiscard]] ProjectionType GetProjectionType() const noexcept { return m_ProjectionType; }

		void SetPerspective(float verticalFOV, float nearClip, float farClip);

		[[nodiscard]] float GetPerspectiveVerticalFOV() const noexcept { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float fov)
		{
			m_PerspectiveFOV = fov;
			RecalculateProjection();
		}

		[[nodiscard]] float GetPerspectiveNearClip() const noexcept { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip)
		{
			m_PerspectiveNear = nearClip;
			RecalculateProjection();
		}

		[[nodiscard]] float GetPerspectiveFarClip() const noexcept { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip)
		{
			m_PerspectiveFar = farClip;
			RecalculateProjection();
		}

		void SetOrthographic(float size, float nearClip, float farClip);

		[[nodiscard]] float GetOrthographicSize() const noexcept { return m_OrthographicSize; }
		void SetOrthographicSize(float size)
		{
			m_OrthographicSize = size;
			RecalculateProjection();
		}

		[[nodiscard]] float GetOrthographicNearClip() const noexcept { return m_OrthographicNear; }
		void SetOrthographicNearClip(float nearClip)
		{
			m_OrthographicNear = nearClip;
			RecalculateProjection();
		}

		[[nodiscard]] float GetOrthographicFarClip() const noexcept { return m_OrthographicFar; }
		void SetOrthographicFarClip(float farClip)
		{
			m_OrthographicFar = farClip;
			RecalculateProjection();
		}

	private:
		void RecalculateProjection();

	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		static constexpr float DEFAULT_ORTHOGRAPHIC_SIZE = 10.0f;
		static constexpr float DEFAULT_PERSPECTIVE_NEAR = 0.01f;
		static constexpr float DEFAULT_PERSPECTIVE_FAR = 1000.0f;
		static constexpr float DEFAULT_ASPECT_RATIO = 16.0f / 9.0f;

		float m_OrthographicSize = DEFAULT_ORTHOGRAPHIC_SIZE;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_PerspectiveFOV = glm::quarter_pi<float>();
		float m_PerspectiveNear = DEFAULT_PERSPECTIVE_NEAR, m_PerspectiveFar = DEFAULT_PERSPECTIVE_FAR;

		float m_AspectRatio = DEFAULT_ASPECT_RATIO;
	};

}
