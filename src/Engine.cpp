#include <Engine.h>
#include <iostream>
namespace TF
{
	Engine::Engine()
	{
		std::cout << "Engine constructor" << std::endl;
	}

	void Engine::Run()
	{
		Init();

		while (false)
		{
			
			Tick(0);
		}

		Terminate();
	}

	void Engine::Tick(double dt)
	{
	}

	void Engine::Init()
	{
		std::cout << "Engine initilization" << std::endl;
	}


	void Engine::Terminate()
	{
		std::cout << "Engine termination" << std::endl;
	}

	Engine::~Engine()
	{
		std::cout << "Engine destructor" << std::endl;
	}

}
