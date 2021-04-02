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
		enum Enum : u32
		{
			StartMaximized = 1,
			EnableVSync = 2,
		};
	};

	FullScreenMode m_FullscreenMode;
	string	m_WindowTitle;
	u32		m_WindowWidth;
	u32		m_WindowHeight;
	u32 m_WindowFlags;
	//bool	m_EnableDebugRendering = false;
};
