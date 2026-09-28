#include "rdpch.h"
#include "Renderer.h"
#include "Renderer2D.h"

#include "Rod/Renderer/RenderCommand.h"

namespace Rod {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();


	struct SceneMatrices
	{
		glm::mat4 ViewProjection;
		glm::mat4 Transform;
	};

	struct DirectionalLight 
	{
		glm::vec3 Direction;
		uint32_t _padding;
		glm::vec3 Color;
		float Intensity;

	};

	constexpr size_t MAX_LIGHT_COUNT = 4;
	struct LightsData
	{
		uint32_t Count;
		uint32_t _padding[3];
		DirectionalLight Lights[MAX_LIGHT_COUNT];
	};

	struct MaterialData
	{
		glm::vec4 Albedo;
		glm::vec3 Emissive;
		float _Padding0;
	};

	struct ShadowData
	{
		glm::mat4 LightSpaceMatrices[MAX_LIGHT_COUNT];
	};

	struct DrawData
	{
		int EntityID;
		int _Padding[3];
	};
	
	namespace Utils {
		constexpr float MAX_SHADOW_DISTANCE = 100.0f;
		constexpr float SHADOW_DEPTH_MARGIN = 10.0f;
		constexpr float SHADOW_SLOPE_SCALE_BIAS = 2.0f;
		constexpr float SHADOW_CONSTANT_BIAS = 4.0f;

		static inline std::array<glm::vec3, 8> GetFrustumCornersWorldSpace(const glm::mat4& viewProjection)
		{
			const glm::mat4 inv =
				glm::inverse(viewProjection);

			std::array<glm::vec3, 8> corners;

			int index = 0;

			for (int x = 0; x < 2; x++)
			{
				for (int y = 0; y < 2; y++)
				{
					for (int z = 0; z < 2; z++)
					{
						glm::vec4 pt =
							inv *
							glm::vec4(
								2.0f * x - 1.0f,
								2.0f * y - 1.0f,
								2.0f * z - 1.0f,
								1.0f);

						corners[index++] =
							glm::vec3(pt) / pt.w;
					}
				}
			}

			return corners;
		}

		static inline void LimitFrustumDepth(std::array<glm::vec3, 8>& corners, float maxDistance)
		{
			glm::vec3 nearCenter(0.0f);
			glm::vec3 farCenter(0.0f);

			for (size_t i = 0; i < corners.size(); i++)
			{
				if (i % 2 == 0)
					nearCenter += corners[i];
				else
					farCenter += corners[i];
			}

			nearCenter *= 0.25f;
			farCenter *= 0.25f;

			glm::vec3 viewDirection = farCenter - nearCenter;
			if (glm::dot(viewDirection, viewDirection) <= 0.0001f)
				return;

			viewDirection = glm::normalize(viewDirection);

			for (auto& corner : corners)
			{
				float depth = glm::dot(corner - nearCenter, viewDirection);
				if (depth > maxDistance)
					corner += viewDirection * (maxDistance - depth);
			}
		}

		static inline SceneMatrices CreateDirectionalLightMatrices(const glm::vec3& lightDirection, const glm::mat4& cameraViewProjection)
		{
			SceneMatrices matrices{};

			auto corners = GetFrustumCornersWorldSpace(cameraViewProjection);
			LimitFrustumDepth(corners, MAX_SHADOW_DISTANCE);

			glm::vec3 center(0.0f);

			for (const auto& c : corners)
				center += c;

			center /= corners.size();

			glm::vec3 normalizedLightDirection = glm::dot(lightDirection, lightDirection) > 0.0001f
				? glm::normalize(lightDirection)
				: glm::vec3(0.0f, -1.0f, 0.0f);
			glm::vec3 lightPos =
				center - normalizedLightDirection * 100.0f;

			glm::vec3 up = glm::abs(glm::dot(normalizedLightDirection, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.95f
				? glm::vec3(0.0f, 0.0f, 1.0f)
				: glm::vec3(0.0f, 1.0f, 0.0f);

			glm::mat4 lightView = glm::lookAt(
				lightPos,
				center,
				up);

			glm::vec3 minBounds(FLT_MAX);
			glm::vec3 maxBounds(-FLT_MAX);

			for (const auto& corner : corners)
			{
				glm::vec4 trf =
					lightView * glm::vec4(corner, 1.0f);

				minBounds = glm::min(minBounds, glm::vec3(trf));
				maxBounds = glm::max(maxBounds, glm::vec3(trf));
			}

			float nearPlane = -maxBounds.z - SHADOW_DEPTH_MARGIN;
			float farPlane = -minBounds.z + SHADOW_DEPTH_MARGIN;
			if (farPlane <= nearPlane)
				farPlane = nearPlane + SHADOW_DEPTH_MARGIN;

			glm::mat4 lightProj = glm::ortho(
				minBounds.x,
				maxBounds.x,
				minBounds.y,
				maxBounds.y,
				nearPlane,
				farPlane);

			matrices.ViewProjection =
				lightProj * lightView;

			glm::mat4 bias(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f);

			matrices.Transform =
				bias * matrices.ViewProjection;

			return matrices;
		}
	}

	void Renderer::Init()
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();

		s_SceneData->Shader = Shader::Create("assets/shaders/3D.glsl");
		s_SceneData->ShadowShader = Shader::Create("assets/shaders/Shadow.glsl");

		std::array<int, MAX_LIGHT_COUNT> shadowMapSamplers{};
		for (uint32_t i = 0; i < MAX_LIGHT_COUNT; i++)
			shadowMapSamplers[i] = static_cast<int>(i);

		s_SceneData->Shader->Bind();
		s_SceneData->Shader->SetIntArray("u_ShadowMaps", shadowMapSamplers.data(), (uint32_t)shadowMapSamplers.size());

		s_SceneData->SceneUBO = UniformBuffer::Create(sizeof(SceneMatrices));
		s_SceneData->MaterialUBO = UniformBuffer::Create(sizeof(MaterialData));
		s_SceneData->LightsUBO = UniformBuffer::Create(sizeof(LightsData));
		s_SceneData->DrawUBO = UniformBuffer::Create(sizeof(DrawData));
		s_SceneData->ShadowUBO = UniformBuffer::Create(sizeof(ShadowData));
	}

	void Renderer::Shutdown()
	{
		RD_PROFILE_FUNCTION();

		Renderer2D::Shutdown();
		delete s_SceneData;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform, std::vector<DirectionalLightComponent>& lights)
	{
		RD_PROFILE_FUNCTION();

		s_SceneData->ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_SceneData->DrawCommands.clear();
		SetupScene(lights);
	}

	void Renderer::BeginScene(const EditorCamera& camera, std::vector<DirectionalLightComponent>& lights)
	{
		RD_PROFILE_FUNCTION();

		s_SceneData->ViewProjection = camera.GetViewProjection();
		s_SceneData->DrawCommands.clear();
		SetupScene(lights);
	}

	void Renderer::EndScene()
	{
		DepthPass();
		GeometryPass();
		s_SceneData->DrawCommands.clear();
	}

	void Renderer::Submit(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const Ref<Material>& material, int entityID)
	{
		RD_PROFILE_FUNCTION();

		s_SceneData->DrawCommands.push_back({ vertexArray, transform, material, entityID });
	}

	void Renderer::GeometryPass()
	{
		s_SceneData->Shader->Bind();

		for (uint32_t i = 0; i < s_SceneData->LightSpaceMatrices.size() && i < MAX_LIGHT_COUNT; i++)
			s_SceneData->DepthMaps[i]->BindDepthAttachment(i);

		for (const auto& command : s_SceneData->DrawCommands)
		{
			const auto& vertexArray = command.VertexArray;
			const auto& transform = command.Transform;
			const auto& material = command.Material;

			SceneMatrices matrices;
			matrices.ViewProjection = s_SceneData->ViewProjection;
			matrices.Transform = transform;

			s_SceneData->SceneUBO->BindBase(1);
			s_SceneData->SceneUBO->SetData(&matrices, sizeof(SceneMatrices));

			MaterialData materialData;
			if (material)
			{
				materialData.Albedo = material->GetAlbedo();
				materialData.Emissive = material->GetEmissive();
			}
			else
			{
				materialData.Albedo = glm::vec4(0.6f, 0.2f, 0.8f, 1.0f);
				materialData.Emissive = glm::vec3(0.0f);
			}
			s_SceneData->MaterialUBO->BindBase(3);
			s_SceneData->MaterialUBO->SetData(&materialData, sizeof(MaterialData));

			DrawData drawData{};
			drawData.EntityID = command.EntityID;;
			s_SceneData->DrawUBO->BindBase(4);
			s_SceneData->DrawUBO->SetData(&drawData, sizeof(DrawData));

			vertexArray->Bind();
			vertexArray->GetIndexBuffer()->Bind();
			RenderCommand::DrawIndexed(vertexArray);
		}
	}

	void Renderer::SetupScene(std::vector<DirectionalLightComponent>& lights)
	{
		const uint32_t lightCount = (uint32_t)std::min(lights.size(), MAX_LIGHT_COUNT);

		if (s_SceneData->DepthMaps.size() < lightCount)
		{
			for (uint32_t i = (uint32_t)s_SceneData->DepthMaps.size(); i < lightCount; i++) {
				FramebufferSpecification fbspec;
				fbspec.Attachments = { FramebufferTextureFormat::DEPTH24 };
				fbspec.Height = 2048;
				fbspec.Width = 2048;
				s_SceneData->DepthMaps.push_back(Framebuffer::Create(fbspec));
			}
		}

		LightsData lightData{};
		ShadowData shadowData{};
		s_SceneData->LightsDirections.resize(lightCount);
		s_SceneData->LightSpaceMatrices.resize(lightCount);
		for (uint32_t i = 0; i < lightCount; i++)
		{
			DirectionalLight data;
			data.Direction = lights[i].Direction;
			data.Color = lights[i].Color;
			data.Intensity = lights[i].Intensity;
			lightData.Lights[i] = data;

			s_SceneData->LightsDirections[i] = lights[i].Direction;
			s_SceneData->LightSpaceMatrices[i] = Utils::CreateDirectionalLightMatrices(lights[i].Direction, s_SceneData->ViewProjection).ViewProjection;
			shadowData.LightSpaceMatrices[i] = s_SceneData->LightSpaceMatrices[i];
		}
		lightData.Count = lightCount;

		auto lightsDataSize = sizeof(LightsData);
		s_SceneData->LightsUBO->BindBase(2);
		s_SceneData->LightsUBO->SetData(&lightData, lightsDataSize);

		s_SceneData->ShadowUBO->BindBase(5);
		s_SceneData->ShadowUBO->SetData(&shadowData, sizeof(ShadowData));
	}

	void Renderer::DepthPass()
	{
		if (s_SceneData->DepthMaps.empty() || s_SceneData->DrawCommands.empty())
			return;

		auto previousState = RenderCommand::GetState();

		s_SceneData->ShadowShader->Bind();
		RenderCommand::SetDepthBias(Utils::SHADOW_SLOPE_SCALE_BIAS, Utils::SHADOW_CONSTANT_BIAS);

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

				s_SceneData->SceneUBO->BindBase(1);
				s_SceneData->SceneUBO->SetData(&matrices, sizeof(SceneMatrices));

				command.VertexArray->Bind();
				command.VertexArray->GetIndexBuffer()->Bind();
				RenderCommand::DrawIndexed(command.VertexArray);
			}
		}

		RenderCommand::ClearDepthBias();
		RenderCommand::SetState(previousState);
	}
}
