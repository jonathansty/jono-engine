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

    void Add(std::string levelPath);
    std::string GetLevel(int levelNumber);

    int GetAmountOfLevels();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    std::vector<std::string>m_LevelPathsPtr;
    int m_NumberOfLevels = 0;
};

 
