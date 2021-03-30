#pragma once
#include <EngineSystemBase.h>
#include <entt/entt.hpp>
namespace TF
{
class RenderSystem:EngineSystemBase
{
public:
	void Init();
	void Tick(double dt, entt::registry& reg);
	void Terminate();

};

}
