#include "engine.pch.h"
#include "GameSettings.h"
#include "EngineCfg.h"


REGISTER_TYPE("/Types/Core/EngineCfg", EngineCfg);
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