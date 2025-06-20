	#include <Rod.h>

// Temp
#include "Platform/OpenGL/OpenGLShader.h"
//

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
																			
class ExampleLayer : public Rod::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f), m_SquarePosition(0.0f)
	{
		m_SquareVA.reset(Rod::VertexArray::Create());

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Rod::Ref<Rod::VertexBuffer> squareVB;
		squareVB.reset(Rod::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetBufferLayout({
			{ Rod::ShaderDataType::Float3, "a_Position" },
			{ Rod::ShaderDataType::Float2, "a_TextCoord"},
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Rod::Ref<Rod::IndexBuffer> squareIB;
		squareIB.reset(Rod::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string flatColorShaderVertexSrc = R"(
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


		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0f);
			}
		)";

		m_FlatColorShader.reset(Rod::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc));

		std::string textureShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TextCoord;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec2 v_TextCoord;

			void main()
			{
				v_TextCoord = a_TextCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string textureShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec2 v_TextCoord;

			uniform sampler2D u_Texture;

			void main()
			{
				color = texture(u_Texture, v_TextCoord);
			}
		)";

		m_TextureShader.reset(Rod::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));

		m_Texture = Rod::Texture2D::Create("assets/textures/test.png");

		std::dynamic_pointer_cast<Rod::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<Rod::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
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

		Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Rod::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Rod::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Rod::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Rod::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);
		for (int y = 0; y < 20; y++) 
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Rod::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}
		m_Texture->Bind();
		Rod::Renderer::Submit(m_TextureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		Rod::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
	
	void OnEvent(Rod::Event& event) override
	{	
	}

private:
	Rod::Ref<Rod::Shader> m_FlatColorShader, m_TextureShader;
	Rod::Ref<Rod::VertexArray> m_SquareVA;
	Rod::Ref<Rod::Texture2D> m_Texture;

	Rod::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 0.75f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 17.5f;

	glm::vec3 m_SquarePosition;
	float m_SquareMoveSpeed = 0.75f;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
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