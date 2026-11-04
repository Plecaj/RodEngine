#include "rdpch.h"

#include "Renderer3D.h"

#include "Rod/Renderer/RenderCommand.h"

#include <limits>

namespace Rod {

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
		uint32_t Padding[STD140_PADDING_WORD_COUNT];
		DirectionalLight Lights[MAX_LIGHT_COUNT];
	};

	struct Renderer3D::MaterialData
	{
		glm::vec4 Albedo;
		glm::vec3 Emissive;
		float Padding0;
	};

	struct Renderer3D::ShadowData
	{
		glm::mat4 LightSpaceMatrices[MAX_LIGHT_COUNT];
	};

	struct Renderer3D::DrawData
	{
		int EntityID;
		int Padding[STD140_PADDING_WORD_COUNT];
	};

	Renderer3D::FrustumCorners Renderer3D::GetFrustumCornersWorldSpace(const glm::mat4& viewProjection)
	{
		const glm::mat4 inv = glm::inverse(viewProjection);
		FrustumCorners corners;
		size_t index = 0;

		for (int x = 0; x < AXIS_ENDPOINT_COUNT; x++)
		{
			for (int y = 0; y < AXIS_ENDPOINT_COUNT; y++)
			{
				for (int z = 0; z < AXIS_ENDPOINT_COUNT; z++)
				{
					const glm::vec4 pt = inv * glm::vec4(
					                               CLIP_COORDINATE_SPAN * static_cast<float>(x) - 1.0f,
					                               CLIP_COORDINATE_SPAN * static_cast<float>(y) - 1.0f,
					                               CLIP_COORDINATE_SPAN * static_cast<float>(z) - 1.0f,
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
			if (i % AXIS_ENDPOINT_COUNT == 0)
				nearCenter += corners[i];
			else
				farCenter += corners[i];
		}

		nearCenter *= PLANE_CENTER_WEIGHT;
		farCenter *= PLANE_CENTER_WEIGHT;

		const glm::vec3 viewVector = farCenter - nearCenter;
		if (glm::dot(viewVector, viewVector) <= DIRECTION_TOLERANCE)
			return;

		const glm::vec3 viewDirection = glm::normalize(viewVector);

		// Corners are emitted as near/far pairs because z is the innermost loop.
		for (size_t i = 0; i < corners.size(); i += AXIS_ENDPOINT_COUNT)
		{
			const glm::vec3 nearCorner = corners[i];
			const glm::vec3 farOffset = corners[i + 1] - nearCorner;
			const float farDepth = glm::dot(farOffset, viewDirection);

			if (farDepth <= maxDistance || farDepth <= DIRECTION_TOLERANCE)
				continue;

			corners[i + 1] = nearCorner + farOffset * (maxDistance / farDepth);
		}
	}

	glm::mat4 Renderer3D::CreateDirectionalLightViewProjection(
	    const glm::vec3& lightDirection,
	    const glm::mat4& cameraViewProjection)
	{
		auto corners = GetFrustumCornersWorldSpace(cameraViewProjection);
		LimitFrustumDepth(corners, MAX_SHADOW_DISTANCE);

		glm::vec3 center(0.0f);
		for (const auto& c : corners)
			center += c;
		center /= static_cast<float>(corners.size());

		const glm::vec3 normalizedLightDirection = glm::dot(lightDirection, lightDirection) > DIRECTION_TOLERANCE
		                                               ? glm::normalize(lightDirection)
		                                               : glm::vec3(0.0f, -1.0f, 0.0f);

		const glm::vec3 lightPos = center - normalizedLightDirection * LIGHT_VIEW_DISTANCE;
		const glm::vec3 up =
		    glm::abs(glm::dot(normalizedLightDirection, glm::vec3(0.0f, 1.0f, 0.0f))) > LIGHT_UP_PARALLEL_THRESHOLD
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

		const float nearPlane = -maxBounds.z - SHADOW_DEPTH_MARGIN;
		float farPlane = -minBounds.z + SHADOW_DEPTH_MARGIN;
		if (farPlane <= nearPlane)
			farPlane = nearPlane + SHADOW_DEPTH_MARGIN;

		const glm::mat4 lightProj = glm::ortho(minBounds.x, maxBounds.x, minBounds.y, maxBounds.y, nearPlane, farPlane);

		return lightProj * lightView;
	}

	void Renderer3D::Init()
	{
		RD_PROFILE_FUNCTION();

		s_SceneData = CreateScope<SceneData>();
		s_SceneData->Shader = Shader::Create("assets/shaders/3D.glsl");
		s_SceneData->ShadowShader = Shader::Create("assets/shaders/Shadow.glsl");

		std::array<int, MAX_LIGHT_COUNT> shadowMapSamplers{};
		for (uint32_t i = 0; i < MAX_LIGHT_COUNT; i++)
			shadowMapSamplers[i] = static_cast<int>(i);

		s_SceneData->Shader->Bind();
		s_SceneData->Shader->SetIntArray(
		    "u_ShadowMaps",
		    shadowMapSamplers.data(),
		    static_cast<uint32_t>(shadowMapSamplers.size()));

		s_SceneData->SceneUBO = UniformBuffer::Create(sizeof(SceneMatrices));
		s_SceneData->MaterialUBO = UniformBuffer::Create(sizeof(MaterialData));
		s_SceneData->LightsUBO = UniformBuffer::Create(sizeof(LightsData));
		s_SceneData->DrawUBO = UniformBuffer::Create(sizeof(DrawData));
		s_SceneData->ShadowUBO = UniformBuffer::Create(sizeof(ShadowData));
	}

	void Renderer3D::Shutdown()
	{
		RD_PROFILE_FUNCTION();

		s_SceneData.reset();
	}

	void Renderer3D::BeginScene(
	    const Camera& camera,
	    const glm::mat4& transform,
	    std::vector<DirectionalLightComponent>& lights)
	{
		RD_PROFILE_FUNCTION();

		s_SceneData->ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_SceneData->DrawCommands.clear();
		SetupScene(lights);
	}

	void Renderer3D::BeginScene(const EditorCamera& camera, std::vector<DirectionalLightComponent>& lights)
	{
		RD_PROFILE_FUNCTION();

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
		RD_PROFILE_FUNCTION();

		// Commands retain the shared mesh, which owns its GPU buffers.
		if (mesh && mesh->GetVAO())
			s_SceneData->DrawCommands.push_back({mesh, transform, material, entityID});
	}

	void Renderer3D::GeometryPass()
	{
		s_SceneData->Shader->Bind();

		for (uint32_t i = 0; i < s_SceneData->LightSpaceMatrices.size() && i < MAX_LIGHT_COUNT; i++)
			s_SceneData->DepthMaps[i]->BindDepthAttachment(i);

		for (const auto& command : s_SceneData->DrawCommands)
		{
			SceneMatrices matrices;
			matrices.ViewProjection = s_SceneData->ViewProjection;
			matrices.Transform = command.Transform;

			s_SceneData->SceneUBO->BindBase(SCENE_BINDING);
			s_SceneData->SceneUBO->SetData(&matrices, sizeof(SceneMatrices));

			MaterialData materialData{};
			materialData.Albedo = command.Material.GetAlbedo();
			materialData.Emissive = command.Material.GetEmissive();
			s_SceneData->MaterialUBO->BindBase(MATERIAL_BINDING);
			s_SceneData->MaterialUBO->SetData(&materialData, sizeof(MaterialData));

			DrawData drawData{};
			drawData.EntityID = command.EntityID;
			s_SceneData->DrawUBO->BindBase(DRAW_BINDING);
			s_SceneData->DrawUBO->SetData(&drawData, sizeof(DrawData));

			command.Mesh->GetVAO()->Bind();
			command.Mesh->GetVAO()->GetIndexBuffer()->Bind();
			RenderCommand::DrawIndexed(*command.Mesh->GetVAO());
		}
	}

	void Renderer3D::SetupScene(std::vector<DirectionalLightComponent>& lights)
	{
		const auto lightCount = static_cast<uint32_t>(std::min(lights.size(), MAX_LIGHT_COUNT));

		if (s_SceneData->DepthMaps.size() < lightCount)
		{
			auto previousState = RenderCommand::GetState();

			for (auto i = static_cast<uint32_t>(s_SceneData->DepthMaps.size()); i < lightCount; i++)
			{
				FramebufferSpecification fbspec;
				fbspec.Attachments = {FramebufferTextureSpecification{FramebufferTextureFormat::DEPTH24}};
				fbspec.Height = SHADOW_MAP_SIZE;
				fbspec.Width = SHADOW_MAP_SIZE;
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

		s_SceneData->LightsUBO->BindBase(LIGHTS_BINDING);
		s_SceneData->LightsUBO->SetData(&lightData, sizeof(LightsData));

		s_SceneData->ShadowUBO->BindBase(SHADOW_BINDING);
		s_SceneData->ShadowUBO->SetData(&shadowData, sizeof(ShadowData));
	}

	void Renderer3D::DepthPass()
	{
		if (s_SceneData->DepthMaps.empty() || s_SceneData->DrawCommands.empty())
			return;

		auto previousState = RenderCommand::GetState();

		s_SceneData->ShadowShader->Bind();

		for (size_t i = 0; i < s_SceneData->LightSpaceMatrices.size(); i++)
		{
			const auto& fb = s_SceneData->DepthMaps[i];
			fb->Bind();
			RenderCommand::Clear();

			for (const auto& command : s_SceneData->DrawCommands)
			{
				SceneMatrices matrices;
				matrices.ViewProjection = s_SceneData->LightSpaceMatrices[i];
				matrices.Transform = command.Transform;

				s_SceneData->SceneUBO->BindBase(SCENE_BINDING);
				s_SceneData->SceneUBO->SetData(&matrices, sizeof(SceneMatrices));

				command.Mesh->GetVAO()->Bind();
				command.Mesh->GetVAO()->GetIndexBuffer()->Bind();
				RenderCommand::DrawIndexed(*command.Mesh->GetVAO());
			}
		}

		RenderCommand::SetState(previousState);
	}
}
