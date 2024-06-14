#include <Rod.h>

class Sandbox : public Rod::Application
{
public:
	Sandbox() 
	{

	}

	~Sandbox() 
	{

	}

};

Rod::Application* Rod::CreateApplication()
{
	return new Sandbox();
}
