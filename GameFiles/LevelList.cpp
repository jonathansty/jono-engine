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

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
LevelList::LevelList()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

LevelList::~LevelList()
{
	// nothing to destroy
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void LevelList::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void LevelList::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void LevelList::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void LevelList::Add(String levelPath)
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
String LevelList::GetLevel(int numberLevel)
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


