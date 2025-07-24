#pragma once

#include "Rod/Core/layer.h"

#include "Rod/Events/KeyEvent.h"
#include "Rod/Events/MouseEvent.h"
#include "Rod/Events/ApplicationEvent.h"

namespace Rod {

	class ROD_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void SetDarkThemeColors();

		void SetBlockEvents(bool block) { m_BlockEvents = block; };
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};

}