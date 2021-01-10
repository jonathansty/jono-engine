#include "stdafx.h"



//------------------------------------------------------------------------------
// GameSettings class definitions. Encapsulated the user defined game settings
//------------------------------------------------------------------------------
GameSettings::GameSettings() :
m_WindowTitle("Game Engine"),
m_WindowWidth(853),
m_WindowHeight(480),
m_WindowFlags(WindowFlags::EnableVSync | WindowFlags::EnableConsole),
m_FullscreenMode(FullScreenMode::Windowed)
{

}

GameSettings::~GameSettings()
{

}