#pragma once
#include <EngineSystemBase.h>
struct GLFWwindow;
namespace TF
{
	
	class WindowSystem:EngineSystemBase
	{
	public:
		void Init();
		void Tick();
		void Terminate();
		GLFWwindow* GetWindow();
		bool ShouldCloseWindow();

	private:
		GLFWwindow* m_window = nullptr;
		int m_width = 1920;
		int m_height = 1080;
	};
}