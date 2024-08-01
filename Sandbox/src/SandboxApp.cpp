#include <Rod.h>

#include "imgui.h"

class ExampleLayer : public Rod::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f, 0.0f, 0.0f)
	{
		m_VertexArray.reset(Rod::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		m_VertexBuffer.reset(Rod::VertexBuffer::Create(vertices, sizeof(vertices)));

		Rod::BufferLayout layout = {
			{ Rod::ShaderDataType::Float3, "a_Position"},
			{ Rod::ShaderDataType::Float4, "a_Color"}
		};
		m_VertexBuffer->SetBufferLayout(layout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		m_IndexBuffer.reset(Rod::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec4 v_Color;

			void main()
			{
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}

		)";

		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec4 v_Color;
			
			void main()
			{
				color = v_Color;
			}

		)";

		m_Shader.reset(Rod::Shader::Create(vertexSrc, fragmentSrc));
	}

	void OnUpdate(Rod::Timestep ts) override
	{

		if (Rod::Input::IsKeyPressed(Rod::Key::A))
			m_CameraPosition.x -= m_CameraSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::D))
			m_CameraPosition.x += m_CameraSpeed * ts;

		if (Rod::Input::IsKeyPressed(Rod::Key::W))
			m_CameraPosition.y += m_CameraSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::S))
			m_CameraPosition.y -= m_CameraSpeed * ts;

		if (Rod::Input::IsKeyPressed(Rod::Key::Q))
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::E))
			m_CameraRotation += m_CameraRotationSpeed * ts;

		Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Rod::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Rod::Renderer::BeginScene(m_Camera);

		Rod::Renderer::Submit(m_Shader, m_VertexArray);

		Rod::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
	}
	
	void OnEvent(Rod::Event& event) override
	{	
	}
private:
	std::shared_ptr<Rod::Shader> m_Shader;
	std::shared_ptr<Rod::VertexArray> m_VertexArray;
	std::shared_ptr<Rod::VertexBuffer> m_VertexBuffer;
	std::shared_ptr<Rod::IndexBuffer> m_IndexBuffer;

	Rod::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 17.5f;
	float m_CameraSpeed = 0.75f;
};

class Sandbox : public Rod::Application
{
public:
	Sandbox() 
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() 
	{

	}

};

Rod::Application* Rod::CreateApplication()
{
	return new Sandbox();
}