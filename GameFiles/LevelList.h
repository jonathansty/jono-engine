#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
//-----------------------------------------------------
// LevelList Class									
//-----------------------------------------------------
class LevelList //: public ContactListener
{
public:
	LevelList( );
	virtual ~LevelList( );

	// C++11 make the class non-copyable
	LevelList( const LevelList& ) = delete;
	LevelList& operator=( const LevelList& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Add(String levelPath);
    String GetLevel(int levelNumber);

    int GetAmountOfLevels();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    std::vector<String>m_LevelPathsPtr;
    int m_NumberOfLevels = 0;
};

 
