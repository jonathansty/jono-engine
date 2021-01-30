#include "game.stdafx.h"		
#include "LevelList.h"

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
        GameEngine::instance()->message_box(String("The level with number ") + String(numberLevel) + String(" does not exist"));
    }
    level = numberLevel;
    return m_LevelPathsPtr[level];
}
int LevelList::GetAmountOfLevels()
{
    return m_NumberOfLevels;
}


