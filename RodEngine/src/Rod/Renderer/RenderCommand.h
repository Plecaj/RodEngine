#pragma once

#include "RendererAPI.h"

namespace Rod {

	class RenderCommand
	{
	public:
		static void Init() { GetRendererAPI().Init(); }
		[[nodiscard]] static auto GetMaxTextureSlots() -> int { return GetRendererAPI().GetMaxTextureSlots(); }

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			GetRendererAPI().SetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color) { GetRendererAPI().SetClearColor(color); }
		static void Clear() { GetRendererAPI().Clear(); }
		static void SetDepthBias(float slopeScaleBias, float constantBias)
		{
			GetRendererAPI().SetDepthBias(slopeScaleBias, constantBias);
		}
		static void ClearDepthBias() { GetRendererAPI().ClearDepthBias(); }

		static void DrawIndexed(const VertexArray& vertexArray, uint32_t indexCount = 0)
		{
			GetRendererAPI().DrawIndexed(vertexArray, indexCount);
		}

		[[nodiscard]] static auto GetState() -> RendererAPI::RendererState { return GetRendererAPI().GetState(); }
		static void SetState(const RendererAPI::RendererState& state) { GetRendererAPI().SetState(state); }

	private:
		static auto GetRendererAPI() -> RendererAPI&;
	};

}
