#include "rdpch.h"

#include "Renderer3D.h"

#include "RenderDevice.h"
#include "ShaderLoader.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <limits>
#include <numeric>
#include <ranges>

namespace Rod {
	namespace {
		class RestoreRenderTarget
		{
		public:
			explicit RestoreRenderTarget(const RenderDevice& device)
			    : m_Device(device), m_Previous(device.GetRenderTarget())
			{}
			~RestoreRenderTarget() { m_Device.SetRenderTarget(m_Previous); }
			RestoreRenderTarget(const RestoreRenderTarget&) = delete;
			auto operator=(const RestoreRenderTarget&) -> RestoreRenderTarget& = delete;
			RestoreRenderTarget(RestoreRenderTarget&&) = delete;
			auto operator=(RestoreRenderTarget&&) -> RestoreRenderTarget& = delete;

		private:
			const RenderDevice& m_Device;
			RenderDevice::RenderTarget m_Previous;
		};
	}
	struct Renderer3D::SceneMatrices
	{
		glm::mat4 ViewProjection;
		glm::mat4 Transform;
	};

	struct Renderer3D::LightUniform
	{
		glm::vec3 Direction;
		uint32_t Padding;
		glm::vec3 Color;
		float Intensity;
	};

	struct Renderer3D::LightsData
	{
		uint32_t Count;
		std::array<uint32_t, s_Std140PaddingWordCount> Padding;
		std::array<LightUniform, s_MaxLightCount> Lights;
	};

	struct Renderer3D::MaterialData
	{
		glm::vec4 Albedo;
		glm::vec3 Emissive;
		float Padding0;
	};

	struct Renderer3D::ShadowData
	{
		std::array<glm::mat4, s_MaxLightCount> LightSpaceMatrices;
	};

	struct Renderer3D::DrawData
	{
		int EntityID;
		std::array<int, s_Std140PaddingWordCount> Padding;
	};

	auto Renderer3D::GetFrustumCornersWorldSpace(const glm::mat4& viewProjection) -> Renderer3D::FrustumCorners
	{
		const glm::mat4 inv = glm::inverse(viewProjection);
		FrustumCorners corners;
		size_t index = 0;

		for (int x = 0; x < s_AxisEndpointCount; x++)
		{
			for (int y = 0; y < s_AxisEndpointCount; y++)
			{
				for (int z = 0; z < s_AxisEndpointCount; z++)
				{
					const glm::vec4 pt = inv * glm::vec4(
					                               s_ClipCoordinateSpan * static_cast<float>(x) - 1.0f,
					                               s_ClipCoordinateSpan * static_cast<float>(y) - 1.0f,
					                               s_ClipCoordinateSpan * static_cast<float>(z) - 1.0f,
					                               1.0f);

					// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
					corners[index++] = glm::vec3(pt) / pt.w;
				}
			}
		}

		return corners;
	}

	void Renderer3D::LimitFrustumDepth(FrustumCorners& corners, float maxDistance)
	{
		glm::vec3 nearCenter(0.0f);
		glm::vec3 farCenter(0.0f);

		// Corners are near/far pairs
		const auto cornerPairs = corners | std::views::adjacent<2> | std::views::stride(s_AxisEndpointCount);
		for (const auto [nearCorner, farCorner] : cornerPairs)
		{
			nearCenter += nearCorner;
			farCenter += farCorner;
		}

		nearCenter *= s_PlaneCenterWeight;
		farCenter *= s_PlaneCenterWeight;

		const glm::vec3 viewVector = farCenter - nearCenter;
		if (glm::dot(viewVector, viewVector) <= s_DirectionTolerance)
			return;

		const glm::vec3 viewDirection = glm::normalize(viewVector);

		for (auto [nearCorner, farCorner] : cornerPairs)
		{
			const glm::vec3 farOffset = farCorner - nearCorner;
			const float farDepth = glm::dot(farOffset, viewDirection);

			if (farDepth <= maxDistance || farDepth <= s_DirectionTolerance)
				continue;

			farCorner = nearCorner + farOffset * (maxDistance / farDepth);
		}
	}

	auto Renderer3D::CreateDirectionalLightViewProjection(
	    const glm::vec3& lightDirection,
	    const glm::mat4& cameraViewProjection) -> glm::mat4
	{
		auto corners = GetFrustumCornersWorldSpace(cameraViewProjection);
		LimitFrustumDepth(corners, s_MaxShadowDistance);

		glm::vec3 center(0.0f);
		for (const auto& c : corners)
			center += c;
		center /= static_cast<float>(corners.size());

		const glm::vec3 normalizedLightDirection = glm::dot(lightDirection, lightDirection) > s_DirectionTolerance
		                                               ? glm::normalize(lightDirection)
		                                               : glm::vec3(0.0f, -1.0f, 0.0f);

		const glm::vec3 lightPos = center - normalizedLightDirection * s_LightViewDistance;
		const glm::vec3 up =
		    glm::abs(glm::dot(normalizedLightDirection, glm::vec3(0.0f, 1.0f, 0.0f))) > s_LightUpParallelThreshold
		        ? glm::vec3(0.0f, 0.0f, 1.0f)
		        : glm::vec3(0.0f, 1.0f, 0.0f);

		const glm::mat4 lightView = glm::lookAt(lightPos, center, up);
		glm::vec3 minBounds(std::numeric_limits<float>::max());
		glm::vec3 maxBounds(-std::numeric_limits<float>::max());

		for (const auto& corner : corners)
		{
			const glm::vec4 trf = lightView * glm::vec4(corner, 1.0f);
			minBounds = glm::min(minBounds, glm::vec3(trf));
			maxBounds = glm::max(maxBounds, glm::vec3(trf));
		}

		const float nearPlane = -maxBounds.z - s_ShadowDepthMargin;
		float farPlane = -minBounds.z + s_ShadowDepthMargin;
		if (farPlane <= nearPlane)
			farPlane = nearPlane + s_ShadowDepthMargin;

		const glm::mat4 lightProj = glm::ortho(minBounds.x, maxBounds.x, minBounds.y, maxBounds.y, nearPlane, farPlane);

		return lightProj * lightView;
	}

	Renderer3D::Renderer3D(const RenderDevice& device, const ShaderLoader& shaders)
	    : m_Device(device), m_SceneUBO(device.CreateUniformBuffer(sizeof(SceneMatrices))),
	      m_LightsUBO(device.CreateUniformBuffer(sizeof(LightsData))),
	      m_DrawUBO(device.CreateUniformBuffer(sizeof(DrawData))),
	      m_ShadowUBO(device.CreateUniformBuffer(sizeof(ShadowData))),
	      m_MaterialUBO(device.CreateUniformBuffer(sizeof(MaterialData))), m_Shader(shaders.Load("3D.glsl")),
	      m_ShadowShader(shaders.Load("Shadow.glsl"))
	{
		static_assert(sizeof(LightUniform) == sizeof(glm::vec4) * 2);
		static_assert(offsetof(LightsData, Lights) == sizeof(glm::vec4));
		static_assert(sizeof(LightsData) == sizeof(glm::vec4) + sizeof(LightUniform) * s_MaxLightCount);
		static_assert(sizeof(ShadowData) == sizeof(glm::mat4) * s_MaxLightCount);
		static_assert(sizeof(DrawData) == sizeof(glm::vec4));

		std::array<int, s_MaxLightCount> shadowMapSamplers{};
		std::ranges::iota(shadowMapSamplers, 0);
		m_Shader->SetIntArray("u_ShadowMaps", shadowMapSamplers);
	}

	void Renderer3D::Begin(const glm::mat4& viewProjection, std::span<const DirectionalLight> lights)
	{
		m_ViewProjection = viewProjection;
		m_DrawCommands.clear();
		SetupLights(lights);
	}

	void Renderer3D::End()
	{
		DepthPass();
		GeometryPass();
		m_DrawCommands.clear();
	}

	void Renderer3D::Submit(const Ref<Mesh>& mesh, const glm::mat4& transform, const Material& material, int entityID)
	{
		if (mesh)
			m_DrawCommands.push_back(
			    {.Mesh = mesh, .Transform = transform, .Material = material, .EntityID = entityID});
	}

	void Renderer3D::GeometryPass()
	{
		m_Shader->Bind();
		m_LightsUBO->BindBase(s_LightsBinding);
		m_ShadowUBO->BindBase(s_ShadowBinding);

		uint32_t slot = 0;
		for (const auto& depthMap : m_DepthMaps | std::views::take(m_LightSpaceMatrices.size()))
			depthMap->BindDepthAttachment(slot++);

		for (const auto& command : m_DrawCommands)
		{
			const auto vertexArray = command.Mesh->GetVAO();
			if (!vertexArray)
				continue;

			const SceneMatrices matrices{.ViewProjection = m_ViewProjection, .Transform = command.Transform};

			m_SceneUBO->BindBase(s_SceneBinding);
			m_SceneUBO->SetData(std::as_bytes(std::span{&matrices, 1}));

			const MaterialData materialData{
			    .Albedo = command.Material.GetAlbedo(),
			    .Emissive = command.Material.GetEmissive(),
			    .Padding0 = 0.0f};
			m_MaterialUBO->BindBase(s_MaterialBinding);
			m_MaterialUBO->SetData(std::as_bytes(std::span{&materialData, 1}));

			const DrawData drawData{.EntityID = command.EntityID, .Padding = {}};
			m_DrawUBO->BindBase(s_DrawBinding);
			m_DrawUBO->SetData(std::as_bytes(std::span{&drawData, 1}));

			m_Device.DrawIndexed(*vertexArray);
		}
	}

	void Renderer3D::SetupLights(std::span<const DirectionalLight> lights)
	{
		const auto lightCount = static_cast<uint32_t>(std::min(lights.size(), s_MaxLightCount));

		if (m_DepthMaps.size() < lightCount)
		{
			const RestoreRenderTarget restore(m_Device);

			for (auto i = static_cast<uint32_t>(m_DepthMaps.size()); i < lightCount; i++)
			{
				FramebufferSpecification fbspec;
				fbspec.Attachments = {FramebufferTextureSpecification{FramebufferTextureFormat::DEPTH24}};
				fbspec.Height = s_ShadowMapSize;
				fbspec.Width = s_ShadowMapSize;
				m_DepthMaps.push_back(m_Device.CreateFramebuffer(fbspec));
			}
		}

		LightsData lightData{};
		ShadowData shadowData{};
		m_LightSpaceMatrices.resize(lightCount);

		for (auto [light, uniform, lightSpace, shadowMatrix] :
		     std::views::zip(lights, lightData.Lights, m_LightSpaceMatrices, shadowData.LightSpaceMatrices))
		{
			uniform.Direction = light.Direction;
			uniform.Color = light.Color;
			uniform.Intensity = light.Intensity;

			lightSpace = CreateDirectionalLightViewProjection(light.Direction, m_ViewProjection);
			shadowMatrix = lightSpace;
		}
		lightData.Count = lightCount;

		m_LightsUBO->SetData(std::as_bytes(std::span{&lightData, 1}));

		m_ShadowUBO->SetData(std::as_bytes(std::span{&shadowData, 1}));
	}

	void Renderer3D::DepthPass()
	{
		if (m_LightSpaceMatrices.empty() || m_DrawCommands.empty())
			return;

		const RestoreRenderTarget restore(m_Device);

		m_ShadowShader->Bind();

		for (const auto [depthMap, lightSpace] : std::views::zip(m_DepthMaps, m_LightSpaceMatrices))
		{
			depthMap->Bind();
			m_Device.ClearDepth();

			for (const auto& command : m_DrawCommands)
			{
				const auto vertexArray = command.Mesh->GetVAO();
				if (!vertexArray)
					continue;

				const SceneMatrices matrices{.ViewProjection = lightSpace, .Transform = command.Transform};

				m_SceneUBO->BindBase(s_SceneBinding);
				m_SceneUBO->SetData(std::as_bytes(std::span{&matrices, 1}));

				m_Device.DrawIndexed(*vertexArray);
			}
		}
	}
}
