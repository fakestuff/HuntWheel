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
		m_world.Tick(dt, m_reg);
		m_renderSystem.Tick(dt, m_reg);
	}

	void Engine::Init()
	{
		std::cout << "Engine initilization" << std::endl;
		m_timer.Init();
		m_window.Init();
		m_renderSystem.Init(m_window.GetWindow());
		m_world.Init();
		
	}


	void Engine::Terminate()
	{
		std::cout << "Engine termination" << std::endl;
		m_world.Terminate();
		m_renderSystem.Terminate();
		m_window.Terminate();
		m_timer.Terminate();
		

	}

	Engine::~Engine()
	{
		std::cout << "Engine destructor" << std::endl;
	}

}
