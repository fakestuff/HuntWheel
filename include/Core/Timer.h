#pragma once
#include <chrono>
#include <EngineSystemBase.h>
namespace TF
{
	class Timer:EngineSystemBase
	{
	public:
		void Init()
		{
			using namespace std::chrono;
			m_curFrameTime = m_lastFrameTime = high_resolution_clock::now();
		}
		void Tick()
		{
			using namespace std::chrono;
			m_curFrameTime = high_resolution_clock::now();
			m_dt = duration_cast<duration<double>>(m_curFrameTime - m_lastFrameTime).count();
			m_lastFrameTime = m_curFrameTime;
		}
		double DeltaTime()
		{
			return m_dt;
		}
		void Terminate()
		{

		}
	private:
		std::chrono::high_resolution_clock::time_point m_lastFrameTime;
		std::chrono::high_resolution_clock::time_point m_curFrameTime;
		double m_dt = 0;
	};
}