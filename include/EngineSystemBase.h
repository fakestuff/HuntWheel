#pragma once
namespace TF
{
	class EngineSystemBase
	{
		virtual void Init() = 0;
		virtual void Terminate() = 0;
	};
}