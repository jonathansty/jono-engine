#pragma once
#include "EString.h"

class GameSettings
{
public:
	GameSettings(void);
	virtual ~GameSettings(void);

	// C++11 make the class non-copyable
	GameSettings(const GameSettings&) = delete;
	GameSettings& operator=(const GameSettings&) = delete;

	enum class FullScreenMode
	{
		Windowed,
		BorderlessWindowed,
		Fullscreen
	};

	struct WindowFlags
	{
		enum Enum : uint32_t
		{
			StartMaximized = 1,
			EnableConsole = 2,
			EnableVSync = 4,
		};
	};

	FullScreenMode m_FullscreenMode;
	String	m_WindowTitle;
	int		m_WindowWidth;
	int		m_WindowHeight;
	uint32_t m_WindowFlags;
	//bool	m_EnableDebugRendering = false;
};
