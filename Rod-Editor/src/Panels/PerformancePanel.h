#pragma once

#include "Rod.h"

namespace Rod {

	class PerformancePanel {
	public:
		PerformancePanel() = default;
		~PerformancePanel() = default;

		void OnImGuiRender(float deltaTime);
	};

}
