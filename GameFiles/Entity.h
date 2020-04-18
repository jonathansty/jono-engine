#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

#include "ContactListener.h"

class Level;
class Avatar;
class sound_manager;
class Entity : public ContactListener
{
public:

	Entity(DOUBLE2 position);
    Entity(DOUBLE2 position, Level* levelPtr);
	virtual ~Entity( );

	// C++11 make the class non-copyable
	Entity( const Entity& ) = delete;
	Entity& operator=( const Entity& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr) = 0; 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr) = 0;   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse) = 0;
    virtual void SetLevel(Level* levelptr);
    virtual void SetAvatar(Avatar* avatar);
    virtual void Paint() {}
    virtual void Tick(double deltaTime) {}
    virtual PhysicsActor* GetActor();
    virtual DOUBLE2 GetPosition();
    virtual void SetSpawnPosition(DOUBLE2 respawnPosition);
    virtual void Reset() = 0;
    virtual void SetGravityScale(double number);
    virtual bool isHit();
    virtual void SetName(String name);
    virtual String GetName();
    virtual bool GetIsDead();
    virtual void PaintDebug();
    int distance(Entity* otherEntityPtr);
    
protected:
    static const double GRAVITYCOEFF;

    String m_Name;
    Level* m_LevelPtr = nullptr;
    DOUBLE2 m_Position;
    DOUBLE2 m_RespawnPosition;
    double m_GravityScale = 1;
    Avatar* m_AvatarPtr = nullptr;
    PhysicsActor* m_ActPtr = nullptr;
    bool m_IsDead = false;
    bool m_IsHit = false;
};

 
