//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

#pragma once

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
