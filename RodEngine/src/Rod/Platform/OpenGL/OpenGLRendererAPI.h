#pragma once

#include "Rod/Renderer/RendererAPI.h"

namespace Rod {

	class OpenGLRendererAPI : public RendererAPI
	{
		virtual void Init();
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIdexed(const Ref<VertexArray>& vertexArray) override;
	};

}