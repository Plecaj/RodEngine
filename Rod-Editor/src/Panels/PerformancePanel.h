#pragma once

#include "Rod.h"

namespace Rod {

	class PerformancePanel
	{
	public:
		PerformancePanel() = default;

		void OnImGuiRender(float deltaTime);
	};

}
