#include "rdpch.h"
#include "Renderer.h"
#include "Renderer2D.h"

#include "Rod/Renderer/RenderCommand.h"

namespace Rod {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();


	struct ShaderMatrices
	{
		glm::mat4 ViewProjection;
		glm::mat4 Transform;
	};

	void Renderer::Init()
	{
		RD_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();

		s_SceneData->SceneUBO = UniformBuffer::Create(sizeof(ShaderMatrices));
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

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		RD_PROFILE_FUNCTION();

		s_SceneData->ViewProjection = camera.GetProjection() * glm::inverse(transform);

	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		RD_PROFILE_FUNCTION();

		s_SceneData->ViewProjection = camera.GetViewProjection();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		RD_PROFILE_FUNCTION();

		ShaderMatrices matrices;
		matrices.ViewProjection = s_SceneData->ViewProjection;
		matrices.Transform = transform;

		s_SceneData->SceneUBO->BindBase(1);
		s_SceneData->SceneUBO->SetData(&matrices, sizeof(ShaderMatrices));

		shader->Bind();
		vertexArray->Bind();
		vertexArray->GetIndexBuffer()->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}
