#pragma once

#include "Rod/Renderer/Camera.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Renderer/FrameBuffer.h"
#include "Rod/Renderer/Shader.h"
#include "Rod/Scene/Components.h"

namespace Rod {

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform, std::vector<DirectionalLightComponent>& lights);
		static void BeginScene(const EditorCamera& camera, std::vector<DirectionalLightComponent>& lights);
		static void EndScene();

		static void Submit(const Ref<Mesh>& mesh, const glm::mat4& transform, const Material& material, int entityID = -1);

	private:
		static void SetupScene(std::vector<DirectionalLightComponent>& lights);
		static void DepthPass();
		static void GeometryPass();

	private:
		struct DrawCommand
		{
			Ref<Rod::Mesh> Mesh;
			glm::mat4 Transform = glm::mat4(1.0f);
			Rod::Material Material;
			int EntityID = -1;
		};

		struct SceneData
		{
			glm::mat4 ViewProjection;
			Scope<UniformBuffer> SceneUBO, LightsUBO, DrawUBO, ShadowUBO, MaterialUBO;
			std::vector<glm::vec3> LightsDirections;
			std::vector<glm::mat4> LightSpaceMatrices;
			Scope<Shader> Shader, ShadowShader;
			std::vector<Scope<Framebuffer>> DepthMaps;
			std::vector<DrawCommand> DrawCommands;
		};

		static Scope<SceneData> s_SceneData;
	};

}
