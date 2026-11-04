#include "rdpch.h"

#include "EditorCamera.h"

#include "Rod/Core/Input.h"
#include "Rod/Core/KeyCodes.h"
#include "Rod/Core/MouseButtonCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Rod {

	float EditorCamera::PanFactor(float viewportSize)
	{
		const float factor = std::min(viewportSize / PAN_VIEWPORT_SCALE, MAX_PAN_VIEWPORT_FACTOR);
		return PAN_QUADRATIC_COEFFICIENT * (factor * factor) - PAN_LINEAR_COEFFICIENT * factor +
		       PAN_CONSTANT_COEFFICIENT;
	}

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
	    : Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)), m_FOV(fov),
	      m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		m_Position = CalculatePosition();

		const glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		return {PanFactor(m_ViewportWidth), PanFactor(m_ViewportHeight)};
	}

	float EditorCamera::RotationSpeed() const
	{
		return ROTATION_SPEED;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * ZOOM_DISTANCE_SCALE;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, MAX_ZOOM_SPEED);
		return speed;
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		(void)ts;

		if (Input::IsKeyPressed(Key::LeftAlt) && m_ControlsEnabled)
		{
			const glm::vec2& mouse{Input::GetMouseX(), Input::GetMouseY()};
			const glm::vec2 delta = (mouse - m_InitialMousePosition) * MOUSE_DELTA_SCALE;
			m_InitialMousePosition = mouse;

			if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
				MouseZoom(delta.y);
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(RD_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		const float delta = e.GetYOffset() * SCROLL_DELTA_SCALE;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return {glm::vec3(-m_Pitch, -m_Yaw, 0.0f)};
	}

}
