#pragma once

#include "RendererAPI.h"
#include "Rod/Core/Core.h"

namespace Rod {

	class RenderCommand
	{
	public:
		static void Init() { s_RendererAPI->Init(); }
		[[nodiscard]] static int GetMaxTextureSlots() { return s_RendererAPI->GetMaxTextureSlots(); }

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
		static void Clear() { s_RendererAPI->Clear(); }
		static void SetDepthBias(float slopeScaleBias, float constantBias)
		{
			s_RendererAPI->SetDepthBias(slopeScaleBias, constantBias);
		}
		static void ClearDepthBias() { s_RendererAPI->ClearDepthBias(); }

		static void DrawIndexed(const VertexArray& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		[[nodiscard]] static RendererAPI::RendererState GetState() { return s_RendererAPI->GetState(); }
		static void SetState(const RendererAPI::RendererState& state) { s_RendererAPI->SetState(state); }

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
