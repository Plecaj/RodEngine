#pragma once
#include "RenderCommand.h"

#include <stdint.h>

namespace Rod{

	class RendererLimits
	{
	public:
		static const uint32_t GetMaxTextureSlots() { return RenderCommand::GetMaxTextureSlots(); };
	};

}