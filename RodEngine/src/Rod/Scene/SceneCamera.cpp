#include "rdpch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Rod {

	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		SetProjectionType(ProjectionType::Perspective);
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farCLip)
	{
		SetProjectionType(ProjectionType::Orthographic);
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farCLip;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective) 
		{
			m_Projection = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		}
		else 
		{
			float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoBottom = -m_OrthographicSize * 0.5f;
			float orthoTop = m_OrthographicSize * 0.5f;

			m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
		}
	}

}