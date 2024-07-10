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
	}

	~Sandbox() 
	{

	}

};

Rod::Application* Rod::CreateApplication()
{
	return new Sandbox();
}
