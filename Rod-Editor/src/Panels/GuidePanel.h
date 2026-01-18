#pragma once

#include "Rod.h"

namespace Rod {

	class GuidePanel {
	public:
		GuidePanel() = default;
		~GuidePanel() = default;

		void OnImGuiRender();
	};

}
