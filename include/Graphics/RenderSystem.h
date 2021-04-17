#pragma once
#include <EngineSystemBase.h>
#include <entt/entt.hpp>
#include <Graphics/GfxDevice.h>
struct GLFWwindow;
namespace TF
{
class RenderSystem:EngineSystemBase
{
public:
	void Init(GLFWwindow* window);
	void Tick(double dt, entt::registry& reg);
	void Terminate();
	TF::GFX::GfxVkDevice* m_gfxVkDevice;

};

}
