#pragma once
#include "RenderCommand.h"

#include <cstdint>

namespace Rod {

	class RendererLimits
	{
	public:
		[[nodiscard]] static uint32_t GetMaxTextureSlots() { return RenderCommand::GetMaxTextureSlots(); };
	};

}