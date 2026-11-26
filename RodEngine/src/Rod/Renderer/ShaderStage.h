#pragma once

#include <cstdint>
#include <vector>

namespace Rod {

	enum class ShaderStage : uint8_t
	{
		Vertex,
		Fragment,
		Geometry,
		TessellationControl,
		TessellationEvaluation,
		Compute,
	};

	struct CompiledShaderStage
	{
		ShaderStage Stage;
		std::vector<uint32_t> Words;
	};

}
