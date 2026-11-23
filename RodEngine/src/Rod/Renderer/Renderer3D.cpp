#include "rdpch.h"

#include "Renderer3D.h"

#include "Rod/Renderer/RenderCommand.h"
#include "Rod/Renderer/Renderer.h"

#include <limits>
#include <optional>

namespace Rod {
	namespace {

		void DrawMesh(const Mesh& mesh)
		{
			const auto vertexArray = mesh.GetVAO();
			if (!vertexArray)
				throw std::bad_optional_access{};
			const auto indexBuffer = vertexArray->GetIndexBuffer();
			if (!indexBuffer)
				throw std::bad_optional_access{};
			vertexArray->Bind();
			indexBuffer->Bind();
			RenderCommand::DrawIndexed(*vertexArray);
		}

	}

	Scope<Renderer3D::SceneData> Renderer3D::s_SceneData;

	struct Renderer3D::SceneMatrices
	{
		glm::mat4 ViewProjection;
		glm::mat4 Transform;
	};

	struct Renderer3D::DirectionalLight
	{
		glm::vec3 Direction;
		uint32_t Padding;
		glm::vec3 Color;
		float Intensity;
	};

	struct Renderer3D::LightsData
	{
		uint32_t Count;
		uint32_t Padding[s_Std140PaddingWordCount];
		DirectionalLight Lights[s_MaxLightCount];
	};

	struct Renderer3D::MaterialData
	{
		glm::vec4 Albedo;
		glm::vec3 Emissive;
		float Padding0;
	};

	struct Renderer3D::ShadowData
	{
		glm::mat4 LightSpaceMatrices[s_MaxLightCount];
	};

	struct Renderer3D::DrawData
	{
		int EntityID;
		int Padding[s_Std140PaddingWordCount];
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

		for (size_t i = 0; i < corners.size(); i++)
		{
			if (i % s_AxisEndpointCount == 0)
				nearCenter += corners[i];
			else
				farCenter += corners[i];
		}

		nearCenter *= s_PlaneCenterWeight;
		farCenter *= s_PlaneCenterWeight;

		const glm::vec3 viewVector = farCenter - nearCenter;
		if (glm::dot(viewVector, viewVector) <= s_DirectionTolerance)
			return;

		const glm::vec3 viewDirection = glm::normalize(viewVector);

		// Corners are emitted as near/far pairs because z is the innermost loop.
		for (size_t i = 0; i < corners.size(); i += s_AxisEndpointCount)
		{
			const glm::vec3 nearCorner = corners[i];
			const glm::vec3 farOffset = corners[i + 1] - nearCorner;
			const float farDepth = glm::dot(farOffset, viewDirection);

			if (farDepth <= maxDistance || farDepth <= s_DirectionTolerance)
				continue;

			corners[i + 1] = nearCorner + farOffset * (maxDistance / farDepth);
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

	void Renderer3D::Init()
	{
		s_SceneData = CreateScope<SceneData>();
		s_SceneData->Shader = Shader::Create((Renderer::GetShaderDirectory() / "3D.glsl").string());
		s_SceneData->ShadowShader = Shader::Create((Renderer::GetShaderDirectory() / "Shadow.glsl").string());

		std::array<int, s_MaxLightCount> shadowMapSamplers{};
		for (uint32_t i = 0; i < s_MaxLightCount; i++)
			shadowMapSamplers[i] = static_cast<int>(i);

		s_SceneData->Shader->Bind();
		s_SceneData->Shader->SetIntArray("u_ShadowMaps", shadowMapSamplers);

		s_SceneData->SceneUBO = UniformBuffer::Create(sizeof(SceneMatrices));
		s_SceneData->MaterialUBO = UniformBuffer::Create(sizeof(MaterialData));
		s_SceneData->LightsUBO = UniformBuffer::Create(sizeof(LightsData));
		s_SceneData->DrawUBO = UniformBuffer::Create(sizeof(DrawData));
		s_SceneData->ShadowUBO = UniformBuffer::Create(sizeof(ShadowData));
	}

	void Renderer3D::Shutdown()
	{
		s_SceneData.reset();
	}

	void Renderer3D::BeginScene(
	    const Camera& camera,
	    const glm::mat4& transform,
	    std::span<const DirectionalLightComponent> lights)
	{
		s_SceneData->ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_SceneData->DrawCommands.clear();
		SetupScene(lights);
	}

	void Renderer3D::BeginScene(const EditorCamera& camera, std::span<const DirectionalLightComponent> lights)
	{
		s_SceneData->ViewProjection = camera.GetViewProjection();
		s_SceneData->DrawCommands.clear();
		SetupScene(lights);
	}

	void Renderer3D::EndScene()
	{
		DepthPass();
		GeometryPass();
		s_SceneData->DrawCommands.clear();
	}

	void Renderer3D::Submit(const Ref<Mesh>& mesh, const glm::mat4& transform, const Material& material, int entityID)
	{
		// Commands retain the shared mesh, which owns its GPU buffers.
		if (mesh && mesh->GetVAO())
			s_SceneData->DrawCommands.push_back(
			    {.Mesh = mesh, .Transform = transform, .Material = material, .EntityID = entityID});
	}

	void Renderer3D::GeometryPass()
	{
		s_SceneData->Shader->Bind();

		for (uint32_t i = 0; i < s_SceneData->LightSpaceMatrices.size() && i < s_MaxLightCount; i++)
			s_SceneData->DepthMaps[i]->BindDepthAttachment(i);

		for (const auto& command : s_SceneData->DrawCommands)
		{
			const SceneMatrices matrices{.ViewProjection = s_SceneData->ViewProjection, .Transform = command.Transform};

			s_SceneData->SceneUBO->BindBase(s_SceneBinding);
			s_SceneData->SceneUBO->SetData(std::as_bytes(std::span{&matrices, 1}));

			const MaterialData materialData{
			    .Albedo = command.Material.GetAlbedo(),
			    .Emissive = command.Material.GetEmissive(),
			    .Padding0 = 0.0f};
			s_SceneData->MaterialUBO->BindBase(s_MaterialBinding);
			s_SceneData->MaterialUBO->SetData(std::as_bytes(std::span{&materialData, 1}));

			const DrawData drawData{.EntityID = command.EntityID, .Padding = {}};
			s_SceneData->DrawUBO->BindBase(s_DrawBinding);
			s_SceneData->DrawUBO->SetData(std::as_bytes(std::span{&drawData, 1}));

			DrawMesh(*command.Mesh);
		}
	}

	void Renderer3D::SetupScene(std::span<const DirectionalLightComponent> lights)
	{
		const auto lightCount = static_cast<uint32_t>(std::min(lights.size(), s_MaxLightCount));

		if (s_SceneData->DepthMaps.size() < lightCount)
		{
			const auto previousState = RenderCommand::GetState();

			for (auto i = static_cast<uint32_t>(s_SceneData->DepthMaps.size()); i < lightCount; i++)
			{
				FramebufferSpecification fbspec;
				fbspec.Attachments = {FramebufferTextureSpecification{FramebufferTextureFormat::DEPTH24}};
				fbspec.Height = s_ShadowMapSize;
				fbspec.Width = s_ShadowMapSize;
				s_SceneData->DepthMaps.push_back(Framebuffer::Create(fbspec));
			}

			RenderCommand::SetState(previousState);
		}

		LightsData lightData{};
		ShadowData shadowData{};
		s_SceneData->LightsDirections.resize(lightCount);
		s_SceneData->LightSpaceMatrices.resize(lightCount);

		for (uint32_t i = 0; i < lightCount; i++)
		{
			DirectionalLight data{};
			data.Direction = lights[i].Direction;
			data.Color = lights[i].Color;
			data.Intensity = lights[i].Intensity;
			lightData.Lights[i] = data;

			s_SceneData->LightsDirections[i] = lights[i].Direction;
			s_SceneData->LightSpaceMatrices[i] =
			    CreateDirectionalLightViewProjection(lights[i].Direction, s_SceneData->ViewProjection);
			shadowData.LightSpaceMatrices[i] = s_SceneData->LightSpaceMatrices[i];
		}
		lightData.Count = lightCount;

		s_SceneData->LightsUBO->BindBase(s_LightsBinding);
		s_SceneData->LightsUBO->SetData(std::as_bytes(std::span{&lightData, 1}));

		s_SceneData->ShadowUBO->BindBase(s_ShadowBinding);
		s_SceneData->ShadowUBO->SetData(std::as_bytes(std::span{&shadowData, 1}));
	}

	void Renderer3D::DepthPass()
	{
		if (s_SceneData->DepthMaps.empty() || s_SceneData->DrawCommands.empty())
			return;

		const auto previousState = RenderCommand::GetState();

		s_SceneData->ShadowShader->Bind();

		for (size_t i = 0; i < s_SceneData->LightSpaceMatrices.size(); i++)
		{
			const auto& fb = s_SceneData->DepthMaps[i];
			fb->Bind();
			RenderCommand::Clear();

			for (const auto& command : s_SceneData->DrawCommands)
			{
				const SceneMatrices matrices{
				    .ViewProjection = s_SceneData->LightSpaceMatrices[i],
				    .Transform = command.Transform};

				s_SceneData->SceneUBO->BindBase(s_SceneBinding);
				s_SceneData->SceneUBO->SetData(std::as_bytes(std::span{&matrices, 1}));

				DrawMesh(*command.Mesh);
			}
		}

		RenderCommand::SetState(previousState);
	}
}
