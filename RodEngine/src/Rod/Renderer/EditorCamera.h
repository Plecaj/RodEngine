#pragma once

#include "Camera.h"
#include "Rod/Core/Timestep.h"
#include "Rod/Events/Event.h"
#include "Rod/Events/MouseEvent.h"

#include <glm/glm.hpp>

#include <utility>

namespace Rod {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() noexcept = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) noexcept;

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		[[nodiscard]] float GetDistance() const noexcept { return m_Distance; }
		void SetDistance(float distance) noexcept { m_Distance = distance; }

		void SetViewportSize(float width, float height) noexcept
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
			UpdateProjection();
		}

		[[nodiscard]] const glm::mat4& GetViewMatrix() const noexcept { return m_ViewMatrix; }
		[[nodiscard]] glm::mat4 GetViewProjection() const noexcept { return m_Projection * m_ViewMatrix; }

		[[nodiscard]] glm::vec3 GetUpDirection() const noexcept;
		[[nodiscard]] glm::vec3 GetRightDirection() const noexcept;
		[[nodiscard]] glm::vec3 GetForwardDirection() const noexcept;
		[[nodiscard]] const glm::vec3& GetPosition() const noexcept { return m_Position; }
		[[nodiscard]] glm::quat GetOrientation() const noexcept;

		[[nodiscard]] float GetPitch() const noexcept { return m_Pitch; }
		[[nodiscard]] float GetYaw() const noexcept { return m_Yaw; }

		void SetControlsEnabled(bool controlsEnabled) noexcept { m_ControlsEnabled = controlsEnabled; };

	private:
		static constexpr float s_RotationSpeed = 0.8f;
		static constexpr float s_ZoomDistanceScale = 0.2f;
		static constexpr float s_MaxZoomSpeed = 100.0f;
		static constexpr float s_MouseDeltaScale = 0.003f;
		static constexpr float s_ScrollDeltaScale = 0.1f;

		void UpdateProjection() noexcept;
		void UpdateView() noexcept;

		bool OnMouseScroll(MouseScrolledEvent& e) noexcept;

		void MousePan(const glm::vec2& delta) noexcept;
		void MouseRotate(const glm::vec2& delta) noexcept;
		void MouseZoom(float delta) noexcept;

		[[nodiscard]] glm::vec3 CalculatePosition() const noexcept;

		[[nodiscard]] std::pair<float, float> PanSpeed() const noexcept;
		[[nodiscard]] float ZoomSpeed() const noexcept;

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

		glm::vec2 m_InitialMousePosition = {0.0f, 0.0f};

		float m_Distance = s_DefaultDistance;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = s_DefaultViewportWidth, m_ViewportHeight = s_DefaultViewportHeight;

		bool m_ControlsEnabled = true;
	};

}
