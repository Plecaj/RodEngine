#pragma once

#include "Rod.h"

namespace Rod {

	class DebugPanel {
	public:
		DebugPanel() = default;
		~DebugPanel() = default;

		void OnImGuiRender(int gizmoType, Entity hoveredEntity, bool& profiling);
	};

}
