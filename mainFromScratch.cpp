#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

class VulkanRendererApp
{
public:
    

    void run()
    {
        initWindow();
        mainLoop();
        cleanup();
    }
private:

    GLFWwindow* window = nullptr;

    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    }
    void initVulkan()
    {

    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window)) 
        {
            glfwPollEvents();
        }
    }

    void cleanup()
    {
         glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main() 
{
    VulkanRendererApp app;
    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    
}