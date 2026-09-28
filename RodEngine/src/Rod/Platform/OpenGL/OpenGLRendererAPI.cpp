#include "rdpch.h"
#include "OpenGLRendererAPI.h"

#include "glad/glad.h"

namespace Rod {
	void OpenGLRendererAPI::Init()
	{
		RD_PROFILE_FUNCTION();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
	}

	const int OpenGLRendererAPI::GetMaxTextureSlots() const
	{
		int TexSlots;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &TexSlots);
		return TexSlots;
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetDepthBias(float slopeScaleBias, float constantBias)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(slopeScaleBias, constantBias);
	}

	void OpenGLRendererAPI::ClearDepthBias()
	{
		glPolygonOffset(0.0f, 0.0f);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	void OpenGLRendererAPI::DrawIdexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	RendererAPI::RendererState OpenGLRendererAPI::GetState() const
	{
		GLint framebuffer = 0;
		GLint viewport[4] = {};
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
		glGetIntegerv(GL_VIEWPORT, viewport);

		RendererState state;
		state.FramebufferID = static_cast<uint32_t>(framebuffer);
		state.ViewportX = viewport[0];
		state.ViewportY = viewport[1];
		state.ViewportWidth = static_cast<uint32_t>(viewport[2]);
		state.ViewportHeight = static_cast<uint32_t>(viewport[3]);
		return state;
	}

	void OpenGLRendererAPI::SetState(const RendererState& state)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, state.FramebufferID);
		glViewport(state.ViewportX, state.ViewportY, state.ViewportWidth, state.ViewportHeight);
	}

}
