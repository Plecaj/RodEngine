#pragma once

#include <glm/glm.hpp>
#include "VertexArray.h"

namespace Rod {

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual const int GetMaxTextureSlots() const = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void SetDepthBias(float slopeScaleBias, float constantBias) = 0;
		virtual void ClearDepthBias() = 0;

		virtual void DrawIdexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

		struct RendererState
		{
			uint32_t FramebufferID = 0;
			int32_t ViewportX = 0;
			int32_t ViewportY = 0;
			uint32_t ViewportWidth = 0;
			uint32_t ViewportHeight = 0;
		};

		virtual RendererState GetState() const = 0;
		virtual void SetState(const RendererState& state) = 0;

		inline static API GetAPI() { return s_API; };
	private:
		static API s_API;
	};

}
