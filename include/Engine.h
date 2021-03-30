#pragma once
#include <entt/entt.hpp>
#include <Core/Timer.h>
#include <Core/WindowSystem.h>
#include <Core/GameWorld.h>
#include <Graphics/RenderSystem.h>
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
	// loop through all the gameplay sub system
	GameWorld m_world;
	RenderSystem m_renderSystem;

};
}