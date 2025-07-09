#include "Sandbox2D.h"
#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	m_SquareVA = Rod::VertexArray::Create();

	float squareVertices[3 * 4] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	Rod::Ref<Rod::VertexBuffer> squareVB;
	squareVB = Rod::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
	squareVB->SetBufferLayout({
		{ Rod::ShaderDataType::Float3, "a_Position" }
		});
	m_SquareVA->AddVertexBuffer(squareVB);

	uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
	Rod::Ref<Rod::IndexBuffer> squareIB;
	squareIB = Rod::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
	m_SquareVA->SetIndexBuffer(squareIB);

	m_FlatColorShader = Rod::Shader::Create("assets/shaders/FlatColor.glsl");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Rod::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Rod::RenderCommand::Clear();

	Rod::Renderer::BeginScene(m_CameraController.GetCamera());

	std::dynamic_pointer_cast<Rod::OpenGLShader>(m_FlatColorShader)->Bind();
	std::dynamic_pointer_cast<Rod::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

	Rod::Renderer::Submit(m_FlatColorShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

	Rod::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Rod::Event& event)
{
	m_CameraController.OnEvent(event);
}
