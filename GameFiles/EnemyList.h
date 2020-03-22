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
// EnemyList Class									
//-----------------------------------------------------
class Enemy;
class Avatar;
class EnemyList //: public ContactListener
{
public:
	EnemyList( );
	virtual ~EnemyList( );

	// C++11 make the class non-copyable
	EnemyList( const EnemyList& ) = delete;
	EnemyList& operator=( const EnemyList& ) = delete;

    void Tick(double deltaTime);
    void Paint();
    void PaintRockets();
    void PaintDebug();

    void Add(Enemy* tmpEnemyPtr);
    void Remove(Enemy* tmpEnemyPtr);
    void RemoveAll();
    void SafeRemoveAll();
    
    Enemy* IsHit();
    void SetAvatar(Avatar* avatarPtr);
    void Reset();
    int GetSize() const;

    Enemy* IsAttackedByAvatar();

private: 
    void RemoveContactListeners();
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    bool m_IsRemoveReset = false;
    int m_NumberOfEnemies = 0;
    std::vector<Enemy *> m_EnemiesPtrArr;
    Avatar* m_AvatarPtr = nullptr;

};

 
