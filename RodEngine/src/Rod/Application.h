#pragma once

#include "Rod/Core.h"

#include "Rod/Window.h"
#include "Rod/LayerStack.h"
#include "Rod/Events/Event.h"
#include "Rod/Events/ApplicationEvent.h"

#include "Rod/ImGui/ImGuiLayer.h"

#include "Renderer/Shader.h"


namespace Rod {

	class ROD_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
		
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; };

		static inline Application& Get() { return *s_Instance; };
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		unsigned int m_VertexArray, m_VertexBuffer, m_IndexBuffer;
		std::unique_ptr<Shader> m_Shader;
	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}


