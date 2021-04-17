#include <Graphics/RenderSystem.h>
#include <GLFW/glfw3.h>
namespace TF
{
	void RenderSystem::Init(GLFWwindow* window)
	{
		m_gfxVkDevice = new TF::GFX::GfxVkDevice(window);
	}
	void RenderSystem::Tick(double dt, entt::registry& reg)
	{
		// view all the renderer and pos rot scale
	}
	void RenderSystem::Terminate()
	{
		delete m_gfxVkDevice;
	}
}