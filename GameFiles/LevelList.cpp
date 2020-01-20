//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "LevelList.h"

#define GAME_ENGINE (GameEngine::GetSingleton())

LevelList::LevelList()
{
}

LevelList::~LevelList()
{
}

void LevelList::Add(std::string levelPath)
{
    for (size_t i = 0; i < m_LevelPathsPtr.size(); i++)
    {
        if (m_LevelPathsPtr[i] == levelPath)
        {
            return;
        }
    }
    m_NumberOfLevels++;
    m_LevelPathsPtr.push_back(levelPath);
}
std::string LevelList::GetLevel(int numberLevel)
{
    int level = numberLevel;
    if (level > m_NumberOfLevels)
    {
        GAME_ENGINE->MessageBox(String("The level with number ") + String(numberLevel) + String(" does not exist"));
    }
    level = numberLevel;
    return m_LevelPathsPtr[level];
}
int LevelList::GetAmountOfLevels()
{
    return m_NumberOfLevels;
}


