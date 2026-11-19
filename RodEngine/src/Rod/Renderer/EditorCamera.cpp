#include "rdpch.h"

#include "EditorCamera.h"

#include "Rod/Core/Input.h"
#include "Rod/Core/KeyCodes.h"
#include "Rod/Core/MouseButtonCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Rod {

	namespace {
		constexpr float g_PanViewportScale = 1000.0f;
		constexpr float g_MaxPanViewportFactor = 2.4f;
		constexpr float g_PanQuadraticCoefficient = 0.0366f;
		constexpr float g_PanLinearCoefficient = 0.1778f;
		constexpr float g_PanConstantCoefficient = 0.3021f;

		[[nodiscard]] constexpr float PanFactor(float viewportSize) noexcept
		{
			const float factor = std::min(viewportSize / g_PanViewportScale, g_MaxPanViewportFactor);
			return g_PanQuadraticCoefficient * (factor * factor) - g_PanLinearCoefficient * factor +
			       g_PanConstantCoefficient;
		}
	}

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) noexcept
	    : Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)), m_FOV(fov),
	      m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection() noexcept
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView() noexcept
	{
		m_Position = CalculatePosition();

		const glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const noexcept
	{
		return {PanFactor(m_ViewportWidth), PanFactor(m_ViewportHeight)};
	}

	float EditorCamera::ZoomSpeed() const noexcept
	{
		float distance = m_Distance * s_ZoomDistanceScale;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, s_MaxZoomSpeed);
		return speed;
	}

	void EditorCamera::OnUpdate(Timestep ts, const Input& input)
	{
		(void)ts;

		if (m_ControlsEnabled && input.IsKeyPressed(Key::LeftAlt))
		{
			const auto [mouseX, mouseY] = input.GetMousePosition();
			const glm::vec2 mouse{mouseX, mouseY};
			const glm::vec2 delta = (mouse - m_InitialMousePosition) * s_MouseDeltaScale;
			m_InitialMousePosition = mouse;

			if (input.IsMouseButtonPressed(Mouse::ButtonMiddle))
				MousePan(delta);
			else if (input.IsMouseButtonPressed(Mouse::ButtonLeft))
				MouseRotate(delta);
			else if (input.IsMouseButtonPressed(Mouse::ButtonRight))
				MouseZoom(delta.y);
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		HandleEvent<MouseScrolledEvent>(e, [this](MouseScrolledEvent& event) { return OnMouseScroll(event); });
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e) noexcept
	{
		const float delta = e.GetYOffset() * s_ScrollDeltaScale;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta) noexcept
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta) noexcept
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * s_RotationSpeed;
		m_Pitch += delta.y * s_RotationSpeed;
	}

	void EditorCamera::MouseZoom(float delta) noexcept
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::GetUpDirection() const noexcept
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const noexcept
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const noexcept
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition() const noexcept
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const noexcept
	{
		return {glm::vec3(-m_Pitch, -m_Yaw, 0.0f)};
	}

}
