#pragma once

#include "Rod/Renderer/Camera.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Renderer/FrameBuffer.h"
#include "Rod/Renderer/Shader.h"
#include "Rod/Scene/Components.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace Rod {

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void
		BeginScene(const Camera& camera, const glm::mat4& transform, std::span<const DirectionalLightComponent> lights);
		static void BeginScene(const EditorCamera& camera, std::span<const DirectionalLightComponent> lights);
		static void EndScene();

		static void
		Submit(const Ref<Mesh>& mesh, const glm::mat4& transform, const Material& material, int entityID = -1);

	private:
		static constexpr size_t s_Std140PaddingWordCount = 3;
		static constexpr size_t s_FrustumCornerCount = 8;
		static constexpr int s_AxisEndpointCount = 2;
		static constexpr float s_ClipCoordinateSpan = 2.0f;
		static constexpr float s_PlaneCenterWeight = 0.25f;
		static constexpr float s_DirectionTolerance = 0.0001f;
		static constexpr float s_LightViewDistance = 100.0f;
		static constexpr float s_LightUpParallelThreshold = 0.95f;
		static constexpr uint32_t s_ShadowMapSize = 2048;
		static constexpr uint32_t s_SceneBinding = 1;
		static constexpr uint32_t s_LightsBinding = 2;
		static constexpr uint32_t s_MaterialBinding = 3;
		static constexpr uint32_t s_DrawBinding = 4;
		static constexpr uint32_t s_ShadowBinding = 5;
		static constexpr size_t s_MaxLightCount = 4;
		static constexpr float s_MaxShadowDistance = 100.0f;
		static constexpr float s_ShadowDepthMargin = 10.0f;

		using FrustumCorners = std::array<glm::vec3, s_FrustumCornerCount>;
		struct SceneMatrices;
		struct DirectionalLight;
		struct LightsData;
		struct MaterialData;
		struct ShadowData;
		struct DrawData;

		[[nodiscard]] static FrustumCorners GetFrustumCornersWorldSpace(const glm::mat4& viewProjection);
		static void LimitFrustumDepth(FrustumCorners& corners, float maxDistance);
		[[nodiscard]] static glm::mat4
		CreateDirectionalLightViewProjection(const glm::vec3& lightDirection, const glm::mat4& cameraViewProjection);

		static void SetupScene(std::span<const DirectionalLightComponent> lights);
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
