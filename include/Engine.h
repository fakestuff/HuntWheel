#pragma once
#include <entt/entt.hpp>
#include <Core/Timer.h>
#include <Core/WindowSystem.h>
namespace TF
{
// Try using as many components as possible in ecs
class Engine
{
public:
	Engine();
	void Run();
	void Tick(double dt);
	void Init();
	void Terminate();
	~Engine();
private:
	entt::registry m_reg;
	Timer m_timer;
	// window system
	WindowSystem m_window;
	// device
	// mesh system
	// texture system

	// world -- game logic related entity

};
}