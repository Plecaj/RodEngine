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
		SceneCamera() noexcept;

		void SetViewportSize(uint32_t width, uint32_t height) noexcept;

		void SetProjectionType(ProjectionType type) noexcept
		{
			m_ProjectionType = type;
			RecalculateProjection();
		}
		[[nodiscard]] ProjectionType GetProjectionType() const noexcept { return m_ProjectionType; }

		void SetPerspective(float verticalFOV, float nearClip, float farClip) noexcept;

		[[nodiscard]] float GetPerspectiveVerticalFOV() const noexcept { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float fov) noexcept
		{
			m_PerspectiveFOV = fov;
			RecalculateProjection();
		}

		[[nodiscard]] float GetPerspectiveNearClip() const noexcept { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip) noexcept
		{
			m_PerspectiveNear = nearClip;
			RecalculateProjection();
		}

		[[nodiscard]] float GetPerspectiveFarClip() const noexcept { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) noexcept
		{
			m_PerspectiveFar = farClip;
			RecalculateProjection();
		}

		void SetOrthographic(float size, float nearClip, float farClip) noexcept;

		[[nodiscard]] float GetOrthographicSize() const noexcept { return m_OrthographicSize; }
		void SetOrthographicSize(float size) noexcept
		{
			m_OrthographicSize = size;
			RecalculateProjection();
		}

		[[nodiscard]] float GetOrthographicNearClip() const noexcept { return m_OrthographicNear; }
		void SetOrthographicNearClip(float nearClip) noexcept
		{
			m_OrthographicNear = nearClip;
			RecalculateProjection();
		}

		[[nodiscard]] float GetOrthographicFarClip() const noexcept { return m_OrthographicFar; }
		void SetOrthographicFarClip(float farClip) noexcept
		{
			m_OrthographicFar = farClip;
			RecalculateProjection();
		}

	private:
		void RecalculateProjection() noexcept;

	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		static constexpr float s_DefaultOrthographicSize = 10.0f;
		static constexpr float s_DefaultPerspectiveNear = 0.01f;
		static constexpr float s_DefaultPerspectiveFar = 1000.0f;
		static constexpr float s_DefaultAspectRatio = 16.0f / 9.0f;

		float m_OrthographicSize = s_DefaultOrthographicSize;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_PerspectiveFOV = glm::quarter_pi<float>();
		float m_PerspectiveNear = s_DefaultPerspectiveNear, m_PerspectiveFar = s_DefaultPerspectiveFar;

		float m_AspectRatio = s_DefaultAspectRatio;
	};

}
