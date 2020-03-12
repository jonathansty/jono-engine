#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

class LevelList 
{
public:
	LevelList( );
	virtual ~LevelList( );

	LevelList( const LevelList& ) = delete;
	LevelList& operator=( const LevelList& ) = delete;

    void Add(std::string levelPath);
    std::string GetLevel(int levelNumber);

    int GetAmountOfLevels();
private: 
    std::vector<std::string>m_LevelPathsPtr;
    int m_NumberOfLevels = 0;
};

 
