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
#include "Entity.h"
//-----------------------------------------------------
// Arrow Class									
//-----------------------------------------------------
class Arrow : public Entity
{
public:
	Arrow(DOUBLE2 position, Bitmap* bmpPtr);
	virtual ~Arrow( );

	// C++11 make the class non-copyable
	Arrow( const Arrow& ) = delete;
	Arrow& operator=( const Arrow& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    
    void Paint();
    void Tick(double deltaTime);
    PhysicsActor* GetActor();
    void SetSpawnPosition(DOUBLE2 respawnPosition){};
    void Reset(){};
    void SetPushPower(double pushPower);
private:
    Bitmap* m_BmpPtr = nullptr;
    PhysicsActor* m_ActBottomTriggerPtr = nullptr;
    double m_PushPower = 1200;
    static int m_InstanceCounter;

    //Sounds
    sound* m_SndJumpPtr = nullptr;

};

 
