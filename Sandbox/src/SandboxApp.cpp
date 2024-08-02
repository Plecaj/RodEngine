#include <Rod.h>

#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Rod::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f), m_SquarePosition(0.0f)
	{
		// Triangle

		m_VertexArray.reset(Rod::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		std::shared_ptr<Rod::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Rod::VertexBuffer::Create(vertices, sizeof(vertices)));

		Rod::BufferLayout layout = {
			{ Rod::ShaderDataType::Float3, "a_Position"},
			{ Rod::ShaderDataType::Float4, "a_Color"}
		};
		vertexBuffer->SetBufferLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<Rod::IndexBuffer> indexBuffer;
		indexBuffer.reset(Rod::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// Square

		m_SquareVA.reset(Rod::VertexArray::Create());

		float squareVertices[3 * 4] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		std::shared_ptr<Rod::VertexBuffer> squareVB;
		squareVB.reset(Rod::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetBufferLayout({
			{ Rod::ShaderDataType::Float3, "a_Position" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Rod::IndexBuffer> squareIB;
		squareIB.reset(Rod::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec4 v_Color;

			void main()
			{
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
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

		std::string whiteShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string whiteShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.9, 0.9, 0.9, 1.0);
			}
		)";

		m_WhiteShader.reset(Rod::Shader::Create(whiteShaderVertexSrc, whiteShaderFragmentSrc));
	}

	void OnUpdate(Rod::Timestep ts) override
	{
		// --- Camera Movement --------------------------------

		if (Rod::Input::IsKeyPressed(Rod::Key::A))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::D))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;

		if (Rod::Input::IsKeyPressed(Rod::Key::W))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::S))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if (Rod::Input::IsKeyPressed(Rod::Key::Q))
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::E))
			m_CameraRotation += m_CameraRotationSpeed * ts;

		// --- Square Movement -------------------------------

		if (Rod::Input::IsKeyPressed(Rod::Key::Left))
			m_SquarePosition.x -= m_SquareMoveSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::Right))
			m_SquarePosition.x += m_SquareMoveSpeed * ts;

		if (Rod::Input::IsKeyPressed(Rod::Key::Up))
			m_SquarePosition.y += m_SquareMoveSpeed * ts;
		else if (Rod::Input::IsKeyPressed(Rod::Key::Down))
			m_SquarePosition.y -= m_SquareMoveSpeed * ts;

		Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Rod::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Rod::Renderer::BeginScene(m_Camera);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_SquarePosition);

		Rod::Renderer::Submit(m_WhiteShader, m_SquareVA, transform);
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

	std::shared_ptr<Rod::Shader> m_WhiteShader;
	std::shared_ptr<Rod::VertexArray> m_SquareVA;

	Rod::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 0.75f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 17.5f;

	glm::vec3 m_SquarePosition;
	float m_SquareMoveSpeed = 0.75f;
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