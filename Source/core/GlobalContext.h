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
	enki::TaskScheduler*    m_TaskScheduler;
	IO::IPlatformIO*        m_PlatformIO;
	class GameEngine*       m_Engine;
	class InputManager*     m_InputManager;
	class AbstractGame*     m_Game;
	class GraphicsThread*   m_GraphicsThread;
	class TypeManager*      m_TypeManager;
	class IRenderInterface* m_RenderInterface;
};
extern CORE_API GlobalContext* GetGlobalContext();

inline class InputManager*		GetInputManager()	{ return GetGlobalContext()->m_InputManager; }
inline class GraphicsThread*	GetGraphicsThread() { return GetGlobalContext()->m_GraphicsThread; }
inline class GameEngine*		GetGameEngine()		{ return GetGlobalContext()->m_Engine; }

inline class IO::IPlatformIO* GetPlatformIO()
{
    assert(GetGlobalContext()->m_PlatformIO);
    return GetGlobalContext()->m_PlatformIO;
}
