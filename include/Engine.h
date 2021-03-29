#pragma once
#include <entt/entt.hpp>
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
	double m_time = 0;
	entt::registry m_reg;
};
}