#pragma once

#include "Rod/Core/Layer.h"

namespace Rod {

	class Window;

	class ImGuiLayer : public Layer
	{
	public:
		explicit ImGuiLayer(const Window& window);
		~ImGuiLayer() override;
		ImGuiLayer(const ImGuiLayer&) = delete;
		auto operator=(const ImGuiLayer&) -> ImGuiLayer& = delete;
		ImGuiLayer(ImGuiLayer&&) = delete;
		auto operator=(ImGuiLayer&&) -> ImGuiLayer& = delete;

		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

		static void Begin();
		void End() const;

		static void SetDarkThemeColors();

		void SetBlockEvents(bool block) noexcept { m_BlockEvents = block; }

	private:
		const Window& m_Window;
		bool m_BlockEvents = true;
	};

}
