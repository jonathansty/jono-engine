#include "engine.pch.h"
#include "GameSettings.h"
#include "EngineCfg.h"

#include "Types/IniStream.h"

REGISTER_TYPE("/Types/Core/GameCfg", GameCfg)

GameCfg::GameCfg() :
m_WindowTitle("Game Engine"),
m_WindowWidth(853),
m_WindowHeight(480),
m_WindowFlags(WindowFlags::EnableVSync),
m_FullscreenMode(FullScreenMode::Windowed)
{

}

GameCfg::~GameCfg()
{

}

SERIALIZE_FN(GameCfg)
{
	SERIALIZE_PROPERTY(WindowTitle);
	SERIALIZE_PROPERTY(WindowWidth);
	SERIALIZE_PROPERTY(WindowHeight);
}
