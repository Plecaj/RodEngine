#pragma once

#include "Rod/Renderer/OrthographicCamera.h"
#include "Rod/Core/Timestep.h"

#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/MouseEvent.h"

namespace Rod{

	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class OrthographicCameraController {
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false); 

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const{ return m_Camera; }
		
		OrthographicCameraBounds& GetBounds() { return m_Bounds; }
		const OrthographicCameraBounds& GetBounds() const { return m_Bounds; }

		float GetZoomLevel() { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; m_CameraTranslationSpeed = m_ZoomLevel;  CalculateView(); }
	private:
		void CalculateView();

		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCameraBounds m_Bounds;
		OrthographicCamera m_Camera;

		bool m_Rotation = false;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;

		float m_CameraTranslationSpeed = 2.0f, m_CameraRotationSpeed = 90.0f;
	};

}