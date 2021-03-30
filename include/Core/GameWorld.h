#pragma once
#include <EngineSystemBase.h>
#include <entt/entt.hpp>
namespace TF
{
	class GameWorld: EngineSystemBase
	{
	public:
		void Init();
		void Tick(double dt, entt::registry &reg);
		void Terminate();

	// input system
	// physics system
	// collision system
	// ...
	};
}