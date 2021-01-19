#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

#include "ContactListener.h"
//-----------------------------------------------------
// Enemy Class									
//-----------------------------------------------------
class Level;
class Avatar;
class Enemy : public ContactListener
{
public:
	Enemy(DOUBLE2 position);
    Enemy(DOUBLE2 position, Level* levelPtr);
	virtual ~Enemy( );

	// C++11 make the class non-copyable
	Enemy( const Enemy& ) = delete;
	Enemy& operator=( const Enemy& ) = delete;

    virtual void Paint(graphics::D2DRenderContext& ctx) = 0;
    virtual void PaintDebug(graphics::D2DRenderContext& ctx);
    virtual void Tick(double deltaTime) = 0;

    virtual void setName(String name);
    virtual void SetLevel(Level* m_LevelPtr);
    virtual void SetAvatar(Avatar* avatarPtr);
    virtual void SetSpawnPosition(DOUBLE2 respawnPosition);
    virtual void Reset();
    virtual void setGravityScale(double number);
    virtual bool IsHit();

    virtual PhysicsActor* GetActor() = 0;
    virtual DOUBLE2 GetPosition();
    virtual String GetName();

    virtual bool GetAttackByAvatar();
    virtual void RemoveContactListener();
    int distance(Enemy* otherEntityPtr);

protected:
    static const double GRAVITYCOEFF;


    Level* m_LevelPtr = nullptr;
    DOUBLE2 m_Position;
    DOUBLE2 m_RespawnPosition;
    double m_GravityScale = 1;
    Avatar* m_AvatarPtr = nullptr;
    PhysicsActor* m_ActPtr = nullptr;
    bool m_IsHit = false;
    String m_Name;
    
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    
};

 
