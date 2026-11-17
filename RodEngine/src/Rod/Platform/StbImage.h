#pragma once

#include "Rod/Core/Core.h"

#include <stb_image.h>

namespace Rod {
	struct StbiImageDeleter
	{
		void operator()(stbi_uc* pixels) const noexcept { stbi_image_free(pixels); }
	};

	using ImagePixels = Scope<stbi_uc, StbiImageDeleter>;
}
