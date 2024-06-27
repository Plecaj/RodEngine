#include <Rod.h>

class ExampleLayer : public Rod::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		RD_INFO("ExampleLayer::Update");
	}
	
	void OnEvent(Rod::Event& event) override
	{
		RD_TRACE("{}", event.ToString());
	}

};

class Sandbox : public Rod::Application
{
public:
	Sandbox() 
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Rod::ImGuiLayer());
	}

	~Sandbox() 
	{

	}

};

Rod::Application* Rod::CreateApplication()
{
	return new Sandbox();
}
