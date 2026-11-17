#pragma once

#include "Rod/Scene/Entity.h"

namespace Rod {

	struct EditorInteractionState
	{
		Entity SelectedEntity;
		Entity HoveredEntity;
		int GizmoType = -1;
		bool ViewportHovered = false;
	};

}
