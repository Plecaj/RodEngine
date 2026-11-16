#pragma once

#include "Rod.h"

namespace Rod {

	class PerformancePanel
	{
	public:
		PerformancePanel() = default;

		static void OnImGuiRender(float deltaTime);
	};

}
