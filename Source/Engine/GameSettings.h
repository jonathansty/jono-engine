#pragma once

class IniStream;

struct GameCfg final
{
	CLASS_BASE();

	GameCfg();
	~GameCfg();
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


	static void Serialize(IniStream& data, GameCfg* config);
};
