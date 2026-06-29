#pragma once

#include "RendererAPI.h"

namespace Rod {

	class RenderCommand
	{
	public:
		inline static void Init() { s_RendererAPI->Init(); }
		inline static const int GetMaxTextureSlots() { return s_RendererAPI->GetMaxTextureSlots(); };
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) { s_RendererAPI->SetViewport(x, y, width, height); }
		inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); };
		inline static void Clear(){ s_RendererAPI->Clear(); }
		inline static void SetDepthBias(float slopeScaleBias, float constantBias) { s_RendererAPI->SetDepthBias(slopeScaleBias, constantBias); }
		inline static void ClearDepthBias() { s_RendererAPI->ClearDepthBias(); }

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0){ s_RendererAPI->DrawIdexed(vertexArray, indexCount); }

		inline static RendererAPI::RendererState GetState() { return s_RendererAPI->GetState(); }
		inline static void SetState(const RendererAPI::RendererState& state) { s_RendererAPI->SetState(state); }
	private:
		static RendererAPI* s_RendererAPI;
	};

}

