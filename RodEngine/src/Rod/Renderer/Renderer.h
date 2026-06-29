#pragma once

#include "RenderCommand.h"
#include "Rod/Renderer/Camera.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Scene/Components.h"
#include "Rod/Renderer/Framebuffer.h"

#include "Shader.h"

namespace Rod {

	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform, std::vector<DirectionalLightComponent>& lights);
		static void BeginScene(const EditorCamera& camera, std::vector<DirectionalLightComponent>& lights);
		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f), const Ref<Material>& material = nullptr, int entityID = -1);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		static void SetupScene(std::vector<DirectionalLightComponent>& lights);
		static void DepthPass();
		static void GeometryPass();
	private:
		struct DrawCommand
		{
			Ref<VertexArray> VertexArray;
			glm::mat4 Transform = glm::mat4(1.0f);
			Ref<Material> Material;
			int EntityID = -1;
		};

		struct SceneData {
			glm::mat4 ViewProjection;
			Ref<UniformBuffer> SceneUBO, LightsUBO, DrawUBO, ShadowUBO, MaterialUBO;
			std::vector<glm::vec3> LightsDirections;
			std::vector<glm::mat4> LightSpaceMatrices;
			Ref<Shader> Shader, ShadowShader;
			std::vector<Ref<Framebuffer>> DepthMaps;
			std::vector<DrawCommand> DrawCommands;
		};

		static SceneData* s_SceneData;
	};

}
