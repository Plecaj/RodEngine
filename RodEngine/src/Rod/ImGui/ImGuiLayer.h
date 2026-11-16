#pragma once

#include "Rod/Core/Layer.h"

namespace Rod {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer() override;
		ImGuiLayer(const ImGuiLayer&) = delete;
		ImGuiLayer& operator=(const ImGuiLayer&) = delete;
		ImGuiLayer(ImGuiLayer&&) = delete;
		ImGuiLayer& operator=(ImGuiLayer&&) = delete;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		static void Begin();
		static void End();

		static void SetDarkThemeColors();

		void SetBlockEvents(bool block) noexcept { m_BlockEvents = block; }

	private:
		bool m_BlockEvents = true;
	};

}
