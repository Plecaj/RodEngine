#pragma once

#include "Rod.h"

namespace Rod {

	class DebugPanel {
	public:
		DebugPanel() = default;
		~DebugPanel() = default;

		void OnImGuiRender(int guizmoType, Entity hoveredEntity, bool& profiling);
	};

}
