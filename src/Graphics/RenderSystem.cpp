#include <Graphics/RenderSystem.h>
#include <GLFW/glfw3.h>
namespace TF
{
	void RenderSystem::Init(GLFWwindow* window)
	{
		m_gfxVkDevice = new GFX::GfxVkDevice(window); // construct instance and physical device
		m_swapChain = new GFX::GfxVkSwapChain(m_gfxVkDevice);
	}
	void RenderSystem::Tick(double dt, entt::registry& reg)
	{
		// view all the renderer and pos rot scale
	}
	void RenderSystem::Terminate()
	{
		delete m_swapChain;
		delete m_gfxVkDevice;
	}
}