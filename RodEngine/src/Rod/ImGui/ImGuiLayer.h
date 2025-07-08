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

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};

}