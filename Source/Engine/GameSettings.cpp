#include "engine.pch.h"
#include "GameSettings.h"

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