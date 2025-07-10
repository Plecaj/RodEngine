#include <Rod.h>
#include "Rod/Core/EntryPoint.h"

// Temp
#include "Rod/Platform/OpenGL/OpenGLShader.h"
//

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
								
#include "Sandbox2D.h"

class ExampleLayer : public Rod::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
	{
		m_SquareVA = Rod::VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Rod::Ref<Rod::VertexBuffer> squareVB;
		squareVB = Rod::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetBufferLayout({
			{ Rod::ShaderDataType::Float3, "a_Position" },
			{ Rod::ShaderDataType::Float2, "a_TextCoord"},
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Rod::Ref<Rod::IndexBuffer> squareIB;
		squareIB = Rod::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
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

		m_FlatColorShader = Rod::Shader::Create("Flat Shader", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Background = Rod::Texture2D::Create("assets/textures/background.png");
		m_Front = Rod::Texture2D::Create("assets/textures/test.png");

		std::dynamic_pointer_cast<Rod::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Rod::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Rod::Timestep ts) override
	{
		m_CameraController.OnUpdate(ts);

		Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Rod::RenderCommand::Clear();

		Rod::Renderer::BeginScene(m_CameraController.GetCamera());

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
		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Background->Bind();
		Rod::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		m_Front->Bind();
		Rod::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

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
		m_CameraController.OnEvent(event);
	}

private:
	Rod::ShaderLibrary m_ShaderLibrary;
	Rod::Ref<Rod::Shader> m_FlatColorShader;
	Rod::Ref<Rod::VertexArray> m_SquareVA;
	Rod::Ref<Rod::Texture2D> m_Background, m_Front;

	Rod::OrthographicCameraController m_CameraController;

	glm::vec3 m_SquarePosition;
	float m_SquareMoveSpeed = 0.75f;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Rod::Application
{
public:
	Sandbox() 
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox() 
	{

	}

};

Rod::Application* Rod::CreateApplication()
{
	return new Sandbox();
}