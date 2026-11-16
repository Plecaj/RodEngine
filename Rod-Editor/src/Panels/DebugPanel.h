#pragma once

#include "Rod.h"

namespace Rod {

	class DebugPanel
	{
	public:
		DebugPanel() = default;

		static void OnImGuiRender(int gizmoType, Entity hoveredEntity, bool& profiling);
	};

}
