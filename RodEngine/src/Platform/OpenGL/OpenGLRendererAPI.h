#pragma once

#include "Renderer/RendererAPI.h"

namespace Rod {

	class OpenGLRendererAPI : public RendererAPI
	{
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIdexed(const Ref<VertexArray>& vertexArray) override;
	};

}