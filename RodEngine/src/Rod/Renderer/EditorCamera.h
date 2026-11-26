#pragma once

#include "Camera.h"

#include <glm/glm.hpp>

#include <utility>

namespace Rod {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() noexcept = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) noexcept;

		void Pan(glm::vec2 delta) noexcept;
		void Orbit(glm::vec2 delta) noexcept;
		void Dolly(float delta) noexcept;
		void Zoom(float scroll) noexcept;

		[[nodiscard]] auto GetDistance() const noexcept -> float { return m_Distance; }
		void SetDistance(float distance) noexcept { m_Distance = distance; }

		void SetViewportSize(float width, float height) noexcept
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
			UpdateProjection();
		}

		[[nodiscard]] auto GetViewMatrix() const noexcept -> const glm::mat4& { return m_ViewMatrix; }
		[[nodiscard]] auto GetViewProjection() const noexcept -> glm::mat4 { return GetProjection() * m_ViewMatrix; }

		[[nodiscard]] auto GetUpDirection() const noexcept -> glm::vec3;
		[[nodiscard]] auto GetRightDirection() const noexcept -> glm::vec3;
		[[nodiscard]] auto GetForwardDirection() const noexcept -> glm::vec3;
		[[nodiscard]] auto GetPosition() const noexcept -> const glm::vec3& { return m_Position; }
		[[nodiscard]] auto GetOrientation() const noexcept -> glm::quat;

		[[nodiscard]] auto GetPitch() const noexcept -> float { return m_Pitch; }
		[[nodiscard]] auto GetYaw() const noexcept -> float { return m_Yaw; }

	private:
		static constexpr float s_RotationSpeed = 0.8f;
		static constexpr float s_ZoomDistanceScale = 0.2f;
		static constexpr float s_MaxZoomSpeed = 100.0f;
		static constexpr float s_MouseDeltaScale = 0.003f;
		static constexpr float s_ScrollDeltaScale = 0.1f;

		void UpdateProjection() noexcept;
		void UpdateView() noexcept;

		void MouseZoom(float delta) noexcept;

		[[nodiscard]] auto CalculatePosition() const noexcept -> glm::vec3;

		[[nodiscard]] auto PanSpeed() const noexcept -> std::pair<float, float>;
		[[nodiscard]] auto ZoomSpeed() const noexcept -> float;

	private:
		static constexpr float s_DefaultFOV = 45.0f;
		static constexpr float s_DefaultAspectRatio = 1.778f;
		static constexpr float s_DefaultNearClip = 0.1f;
		static constexpr float s_DefaultFarClip = 1000.0f;
		static constexpr float s_DefaultDistance = 10.0f;
		static constexpr float s_DefaultViewportWidth = 1280.0f;
		static constexpr float s_DefaultViewportHeight = 720.0f;

		float m_FOV = s_DefaultFOV, m_AspectRatio = s_DefaultAspectRatio;
		float m_NearClip = s_DefaultNearClip, m_FarClip = s_DefaultFarClip;

		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
		glm::vec3 m_FocalPoint = {0.0f, 0.0f, 0.0f};

		float m_Distance = s_DefaultDistance;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = s_DefaultViewportWidth, m_ViewportHeight = s_DefaultViewportHeight;
	};

}
