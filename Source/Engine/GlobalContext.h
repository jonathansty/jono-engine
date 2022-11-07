#pragma once

namespace IO
{
class IPlatformIO;
}

namespace enki
{
class TaskScheduler;
}

struct GlobalContext
{
	enki::TaskScheduler*  m_TaskScheduler;
	IO::IPlatformIO*      m_PlatformIO;
	class GameEngine*     m_Engine;
	class InputManager*   m_InputManager;
	class AbstractGame*   m_Game;
	class GraphicsThread* m_GraphicsThread;
};
ENGINE_API GlobalContext* GetGlobalContext();
