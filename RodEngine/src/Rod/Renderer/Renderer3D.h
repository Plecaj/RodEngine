#pragma once

#include "Rod/Renderer/Camera.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Renderer/FrameBuffer.h"
#include "Rod/Renderer/Shader.h"
#include "Rod/Scene/Components.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace Rod {

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void
		BeginScene(const Camera& camera, const glm::mat4& transform, std::vector<DirectionalLightComponent>& lights);
		static void BeginScene(const EditorCamera& camera, std::vector<DirectionalLightComponent>& lights);
		static void EndScene();

		static void
		Submit(const Ref<Mesh>& mesh, const glm::mat4& transform, const Material& material, int entityID = -1);

	private:
		static constexpr size_t STD140_PADDING_WORD_COUNT = 3;
		static constexpr size_t FRUSTUM_CORNER_COUNT = 8;
		static constexpr int AXIS_ENDPOINT_COUNT = 2;
		static constexpr float CLIP_COORDINATE_SPAN = 2.0f;
		static constexpr float PLANE_CENTER_WEIGHT = 0.25f;
		static constexpr float DIRECTION_TOLERANCE = 0.0001f;
		static constexpr float LIGHT_VIEW_DISTANCE = 100.0f;
		static constexpr float LIGHT_UP_PARALLEL_THRESHOLD = 0.95f;
		static constexpr uint32_t SHADOW_MAP_SIZE = 2048;
		static constexpr uint32_t SCENE_BINDING = 1;
		static constexpr uint32_t LIGHTS_BINDING = 2;
		static constexpr uint32_t MATERIAL_BINDING = 3;
		static constexpr uint32_t DRAW_BINDING = 4;
		static constexpr uint32_t SHADOW_BINDING = 5;
		static constexpr size_t MAX_LIGHT_COUNT = 4;
		static constexpr float MAX_SHADOW_DISTANCE = 100.0f;
		static constexpr float SHADOW_DEPTH_MARGIN = 10.0f;

		using FrustumCorners = std::array<glm::vec3, FRUSTUM_CORNER_COUNT>;
		struct SceneMatrices;
		struct DirectionalLight;
		struct LightsData;
		struct MaterialData;
		struct ShadowData;
		struct DrawData;

		static FrustumCorners GetFrustumCornersWorldSpace(const glm::mat4& viewProjection);
		static void LimitFrustumDepth(FrustumCorners& corners, float maxDistance);
		static glm::mat4
		CreateDirectionalLightViewProjection(const glm::vec3& lightDirection, const glm::mat4& cameraViewProjection);

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
