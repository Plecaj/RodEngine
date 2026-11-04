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
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		[[nodiscard]] float GetDistance() const noexcept { return m_Distance; }
		void SetDistance(float distance) noexcept { m_Distance = distance; }

		void SetViewportSize(float width, float height)
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
			UpdateProjection();
		}

		[[nodiscard]] const glm::mat4& GetViewMatrix() const noexcept { return m_ViewMatrix; }
		[[nodiscard]] glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		[[nodiscard]] glm::vec3 GetUpDirection() const;
		[[nodiscard]] glm::vec3 GetRightDirection() const;
		[[nodiscard]] glm::vec3 GetForwardDirection() const;
		[[nodiscard]] const glm::vec3& GetPosition() const noexcept { return m_Position; }
		[[nodiscard]] glm::quat GetOrientation() const;

		[[nodiscard]] float GetPitch() const noexcept { return m_Pitch; }
		[[nodiscard]] float GetYaw() const noexcept { return m_Yaw; }

		void SetControlsEnabled(bool controlsEnabled) noexcept { m_ControlsEnabled = controlsEnabled; };

	private:
		static constexpr float PAN_VIEWPORT_SCALE = 1000.0f;
		static constexpr float MAX_PAN_VIEWPORT_FACTOR = 2.4f;
		static constexpr float PAN_QUADRATIC_COEFFICIENT = 0.0366f;
		static constexpr float PAN_LINEAR_COEFFICIENT = 0.1778f;
		static constexpr float PAN_CONSTANT_COEFFICIENT = 0.3021f;
		static constexpr float ROTATION_SPEED = 0.8f;
		static constexpr float ZOOM_DISTANCE_SCALE = 0.2f;
		static constexpr float MAX_ZOOM_SPEED = 100.0f;
		static constexpr float MOUSE_DELTA_SCALE = 0.003f;
		static constexpr float SCROLL_DELTA_SCALE = 0.1f;

		[[nodiscard]] static float PanFactor(float viewportSize);
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		[[nodiscard]] glm::vec3 CalculatePosition() const;

		[[nodiscard]] std::pair<float, float> PanSpeed() const;
		[[nodiscard]] float RotationSpeed() const;
		[[nodiscard]] float ZoomSpeed() const;

	private:
		static constexpr float DEFAULT_FOV = 45.0f;
		static constexpr float DEFAULT_ASPECT_RATIO = 1.778f;
		static constexpr float DEFAULT_NEAR_CLIP = 0.1f;
		static constexpr float DEFAULT_FAR_CLIP = 1000.0f;
		static constexpr float DEFAULT_DISTANCE = 10.0f;
		static constexpr float DEFAULT_VIEWPORT_WIDTH = 1280.0f;
		static constexpr float DEFAULT_VIEWPORT_HEIGHT = 720.0f;

		float m_FOV = DEFAULT_FOV, m_AspectRatio = DEFAULT_ASPECT_RATIO;
		float m_NearClip = DEFAULT_NEAR_CLIP, m_FarClip = DEFAULT_FAR_CLIP;

		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
		glm::vec3 m_FocalPoint = {0.0f, 0.0f, 0.0f};

		glm::vec2 m_InitialMousePosition = {0.0f, 0.0f};

		float m_Distance = DEFAULT_DISTANCE;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = DEFAULT_VIEWPORT_WIDTH, m_ViewportHeight = DEFAULT_VIEWPORT_HEIGHT;

		bool m_ControlsEnabled = true;
	};

}