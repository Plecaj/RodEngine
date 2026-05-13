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

	constexpr size_t MAX_LIGHT_COUNT = 64;
	struct LightMatrices
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

	struct DrawData
	{
		int EntityID;
		int _Padding[3];
	};
	
	void Renderer::Init()
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();

		s_SceneData->Shader = Shader::Create("assets/shaders/3D.glsl");
		s_SceneData->SceneUBO = UniformBuffer::Create(sizeof(SceneMatrices));
		s_SceneData->MaterialUBO = UniformBuffer::Create(sizeof(MaterialData));
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

		LightMatrices lightData;
		for (int i = 0; i < lights.size(); i++)
		{
			DirectionalLight data;
			data.Direction = lights[i].Direction;
			data.Color = lights[i].Color;
			data.Intensity = lights[i].Intensity;
			lightData.Lights[i] = data;
		}
		lightData.Count = lights.size();

		auto lightsDataSize = sizeof(LightMatrices);
		s_SceneData->LightsUBO = UniformBuffer::Create(lightsDataSize);
		s_SceneData->LightsUBO->BindBase(2);
		s_SceneData->LightsUBO->SetData(&lightData, lightsDataSize);
	}

	void Renderer::BeginScene(const EditorCamera& camera, std::vector<DirectionalLightComponent>& lights)
	{
		RD_PROFILE_FUNCTION();

		s_SceneData->ViewProjection = camera.GetViewProjection();

		LightMatrices lightData;
		for (int i = 0; i < lights.size(); i++)
		{
			DirectionalLight data;
			data.Direction = lights[i].Direction;
			data.Color = lights[i].Color;
			data.Intensity = lights[i].Intensity;
			lightData.Lights[i] = data;
		}
		lightData.Count = lights.size();

		auto lightsDataSize = sizeof(LightMatrices);
		s_SceneData->LightsUBO = UniformBuffer::Create(lightsDataSize);
		s_SceneData->LightsUBO->BindBase(2);
		s_SceneData->LightsUBO->SetData(&lightData, lightsDataSize);
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const Ref<Material>& material, int entityID)
	{
		RD_PROFILE_FUNCTION();

		SceneMatrices matrices;
		matrices.ViewProjection = s_SceneData->ViewProjection;
		matrices.Transform = transform;

		s_SceneData->SceneUBO->BindBase(1);
		s_SceneData->SceneUBO->SetData(&matrices, sizeof(SceneMatrices));


		s_SceneData->Shader->Bind();
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
		drawData.EntityID = entityID;
		static Ref<UniformBuffer> drawUBO = UniformBuffer::Create(sizeof(DrawData));
		drawUBO->BindBase(4);
		drawUBO->SetData(&drawData, sizeof(DrawData));
		vertexArray->Bind();
		vertexArray->GetIndexBuffer()->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}
