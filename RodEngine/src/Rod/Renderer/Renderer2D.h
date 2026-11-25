#pragma once

#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace Rod {

	class RenderDevice;
	class ShaderLoader;

	class Renderer2D
	{
	public:
		Renderer2D(const RenderDevice& device, const ShaderLoader& shaders);

		void Begin(const glm::mat4& viewProjection);
		void SubmitQuad(
		    const glm::mat4& transform,
		    const Ref<Texture2D>& texture,
		    float tiling,
		    const glm::vec4& tint,
		    int entityID = -1);
		void End();

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
		};

		[[nodiscard]] auto GetStats() const noexcept -> Statistics { return m_Stats; }

	private:
		static constexpr uint32_t s_VerticesPerQuad = 4;
		static constexpr uint32_t s_IndicesPerQuad = 6;
		static constexpr uint32_t s_QuadCapacity = 10000;
		static constexpr uint32_t s_TextureCapacity = 32;

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec4 Tint;
			glm::vec2 UV;
			float TextureSlot;
			float Tiling;
			int EntityID;
		};

		void Flush();
		[[nodiscard]] auto AssignTextureSlot(const Ref<Texture2D>& texture) -> uint32_t;

		const RenderDevice& m_Device;
		Scope<VertexArray> m_VertexArray;
		Scope<UniformBuffer> m_CameraBuffer;
		Scope<Shader> m_Shader;
		std::vector<Vertex> m_Vertices;
		std::vector<Ref<Texture2D>> m_Textures;
		Statistics m_Stats;
	};

}
