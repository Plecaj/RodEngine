#include "rdpch.h"

#include "EditorCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Rod {

	namespace {
		constexpr float g_PanViewportScale = 1000.0f;
		constexpr float g_MaxPanViewportFactor = 2.4f;
		constexpr float g_PanQuadraticCoefficient = 0.0366f;
		constexpr float g_PanLinearCoefficient = 0.1778f;
		constexpr float g_PanConstantCoefficient = 0.3021f;

		[[nodiscard]] constexpr auto PanFactor(float viewportSize) noexcept -> float
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

	auto EditorCamera::PanSpeed() const noexcept -> std::pair<float, float>
	{
		return {PanFactor(m_ViewportWidth), PanFactor(m_ViewportHeight)};
	}

	auto EditorCamera::ZoomSpeed() const noexcept -> float
	{
		float distance = m_Distance * s_ZoomDistanceScale;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, s_MaxZoomSpeed);
		return speed;
	}

	void EditorCamera::Pan(glm::vec2 delta) noexcept
	{
		delta *= s_MouseDeltaScale;
		const auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
		UpdateView();
	}

	void EditorCamera::Orbit(glm::vec2 delta) noexcept
	{
		delta *= s_MouseDeltaScale;
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * s_RotationSpeed;
		m_Pitch += delta.y * s_RotationSpeed;
		UpdateView();
	}

	void EditorCamera::Dolly(float delta) noexcept
	{
		MouseZoom(delta * s_MouseDeltaScale);
		UpdateView();
	}

	void EditorCamera::Zoom(float scroll) noexcept
	{
		MouseZoom(scroll * s_ScrollDeltaScale);
		UpdateView();
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

	auto EditorCamera::GetUpDirection() const noexcept -> glm::vec3
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	auto EditorCamera::GetRightDirection() const noexcept -> glm::vec3
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	auto EditorCamera::GetForwardDirection() const noexcept -> glm::vec3
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	auto EditorCamera::CalculatePosition() const noexcept -> glm::vec3
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	auto EditorCamera::GetOrientation() const noexcept -> glm::quat
	{
		return {glm::vec3(-m_Pitch, -m_Yaw, 0.0f)};
	}

}
