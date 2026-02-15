#pragma once

#include "RenderCommand.h"
#include "Rod/Renderer/Camera.h"
#include "Rod/Renderer/EditorCamera.h"

#include "Shader.h"

namespace Rod {

	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData {
			glm::mat4 ViewProjection;
			Ref<UniformBuffer> SceneUBO;
		};

		static SceneData* s_SceneData;
	};

}