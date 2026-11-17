#pragma once

#include "Rod/Renderer/Camera.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/ScriptableEntity.h"
#include "SubTexture2D.h"
#include "Texture.h"

namespace Rod {

	class Renderer2D
	{
	public:
		static constexpr uint32_t s_QuadVertexCount = 4;
		static constexpr uint32_t s_QuadIndexCount = 6;

		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawQuad(
		    const glm::mat4& transform,
		    const Ref<Texture2D>& texture,
		    float tilingFactor = 1.0f,
		    const glm::vec4& tintColor = glm::vec4(1.0f),
		    int entityID = -1);

		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			[[nodiscard]] constexpr uint32_t GetTotalVertexCount() const noexcept
			{
				return QuadCount * s_QuadVertexCount;
			};
			[[nodiscard]] constexpr uint32_t GetTotalIndexCount() const noexcept
			{
				return QuadCount * s_QuadIndexCount;
			};
		};
		static void ResetStats() noexcept;
		[[nodiscard]] static Statistics GetStats() noexcept;

	private:
		// Initialize
		static void InitShader();
		static void InitSamplers();
		static void InitQuadVertexArray();
		static void InitWhiteTexture();
		static void InitQuadVertexPositions();

		static void BeginBatch();
		static void Flush();
		static void EndBatch();
	};

}
