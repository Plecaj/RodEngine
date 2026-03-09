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

	struct Intensity 
	{
		glm::vec3 Direction;
		uint32_t _padding;
		glm::vec3 Color;
		float Density;

	};

	constexpr size_t MAX_LIGHT_COUNT = 64;
	struct LightMatrices
	{
		uint32_t Count;
		uint32_t _padding[3];
		Intensity Lights[MAX_LIGHT_COUNT];
	};
	
	void Renderer::Init()
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();

		s_SceneData->Shader = Shader::Create("assets/shaders/3D.glsl");
		s_SceneData->SceneUBO = UniformBuffer::Create(sizeof(SceneMatrices));
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
			Intensity data;
			data.Direction = lights[i].Direction;
			data.Color = lights[i].Color;
			data.Density = lights[i].Intensity;
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
			Intensity data;
			data.Direction = lights[i].Direction;
			data.Color = lights[i].Color;
			data.Density = lights[i].Intensity;
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

	void Renderer::Submit(const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		RD_PROFILE_FUNCTION();

		SceneMatrices matrices;
		matrices.ViewProjection = s_SceneData->ViewProjection;
		matrices.Transform = transform;

		s_SceneData->SceneUBO->BindBase(1);
		s_SceneData->SceneUBO->SetData(&matrices, sizeof(SceneMatrices));


		s_SceneData->Shader->Bind();
		vertexArray->Bind();
		vertexArray->GetIndexBuffer()->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}
