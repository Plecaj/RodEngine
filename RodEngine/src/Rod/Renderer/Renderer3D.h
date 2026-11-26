#pragma once

#include "Rod/Renderer/FrameBuffer.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Renderer/Shader.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace Rod {

	class RenderDevice;
	class ShaderLoader;

	class Renderer3D
	{
	public:
		Renderer3D(const RenderDevice& device, const ShaderLoader& shaders);

		struct DirectionalLight
		{
			glm::vec3 Direction;
			glm::vec3 Color;
			float Intensity;
		};

		void Begin(const glm::mat4& viewProjection, std::span<const DirectionalLight> lights);
		void Submit(const Ref<Mesh>& mesh, const glm::mat4& transform, const Material& material, int entityID = -1);
		void End();

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
		struct LightUniform;
		struct LightsData;
		struct MaterialData;
		struct ShadowData;
		struct DrawData;

		[[nodiscard]] static auto GetFrustumCornersWorldSpace(const glm::mat4& viewProjection) -> FrustumCorners;
		static void LimitFrustumDepth(FrustumCorners& corners, float maxDistance);
		[[nodiscard]] static auto
		CreateDirectionalLightViewProjection(const glm::vec3& lightDirection, const glm::mat4& cameraViewProjection)
		    -> glm::mat4;

		void SetupLights(std::span<const DirectionalLight> lights);
		void DepthPass();
		void GeometryPass();

	private:
		struct DrawCommand
		{
			Ref<Rod::Mesh> Mesh;
			glm::mat4 Transform = glm::mat4(1.0f);
			Rod::Material Material;
			int EntityID = -1;
		};

		const RenderDevice& m_Device;
		glm::mat4 m_ViewProjection{};
		Scope<UniformBuffer> m_SceneUBO, m_LightsUBO, m_DrawUBO, m_ShadowUBO, m_MaterialUBO;
		std::vector<glm::mat4> m_LightSpaceMatrices;
		Scope<Shader> m_Shader, m_ShadowShader;
		std::vector<Scope<Framebuffer>> m_DepthMaps;
		std::vector<DrawCommand> m_DrawCommands;
	};

}
