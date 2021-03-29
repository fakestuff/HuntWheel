#include <Core/WindowSystem.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace TF
{
	void WindowSystem::Init()
	{
		glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        //glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}
	void WindowSystem::Tick()
	{
		glfwPollEvents();
	}
	void WindowSystem::Terminate()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
	GLFWwindow* WindowSystem::GetWindow()
	{
		return m_window;
	}
	bool WindowSystem::ShouldCloseWindow()
	{
		return glfwWindowShouldClose(m_window);
	}
}