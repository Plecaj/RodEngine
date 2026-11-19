#pragma once

#include "Rod/Core/Layer.h"

namespace Rod {

	class Window;

	class ImGuiLayer : public Layer
	{
	public:
		explicit ImGuiLayer(const Window& window);
		virtual ~ImGuiLayer() override;
		ImGuiLayer(const ImGuiLayer&) = delete;
		ImGuiLayer& operator=(const ImGuiLayer&) = delete;
		ImGuiLayer(ImGuiLayer&&) = delete;
		ImGuiLayer& operator=(ImGuiLayer&&) = delete;

		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		static void Begin();
		void End() const;

		static void SetDarkThemeColors();

		void SetBlockEvents(bool block) noexcept { m_BlockEvents = block; }

	private:
		const Window& m_Window;
		bool m_BlockEvents = true;
	};

}
