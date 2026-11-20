#pragma once

namespace Rod {

	class GraphicsContext
	{
	public:
		GraphicsContext() = default;
		virtual ~GraphicsContext() = default;
		GraphicsContext(const GraphicsContext&) = delete;
		auto operator=(const GraphicsContext&) -> GraphicsContext& = delete;
		GraphicsContext(GraphicsContext&&) = delete;
		auto operator=(GraphicsContext&&) -> GraphicsContext& = delete;

		virtual void SwapBuffers() = 0;
	};

}
