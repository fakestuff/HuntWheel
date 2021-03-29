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
		int i = 0;
		while (!m_window.ShouldCloseWindow())
		{
			m_timer.Tick();
			double dt = m_timer.DeltaTime();
			std::cout << dt << std::endl;
			Tick(dt);
		}

		Terminate();
	}

	void Engine::Tick(double dt)
	{
		m_window.Tick();
	}

	void Engine::Init()
	{
		std::cout << "Engine initilization" << std::endl;
		m_timer.Init();
		m_window.Init();
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
