#include "engine.pch.h"
#include "GameSettings.h"

GameSettings::GameSettings() :
m_WindowTitle("Game Engine"),
m_WindowWidth(853),
m_WindowHeight(480),
m_WindowFlags(WindowFlags::EnableVSync),
m_FullscreenMode(FullScreenMode::Windowed)
{

}

GameSettings::~GameSettings()
{

}